#include "pch.h"
#include "MotionBlurPass.h"
#include "../SceneView.h"
#include "../../RHI/ConstantBufferType.h"
#include "../../RHI/D3D11RHI.h"
#include "../../RHI/SwapGuard.h"
#include "Source/Runtime/Engine/Components/MotionBlurComponent.h"

const char* FMotionBlurPass::MB_CalcScreenVelocityPSPath = "Shaders/PostProcess/MB_CalcScreenVelocity_PS.hlsl";
const char* FMotionBlurPass::MB_MotionBlurPSPath = "Shaders/PostProcess/MB_MotionBlur_PS.hlsl";

bool FMotionBlurPass::bFirstFrame = true;
FMatrix FMotionBlurPass::LastFrameViewProj = FMatrix::Identity();
void FMotionBlurPass::Execute(const FPostProcessModifier& M, FSceneView* View, D3D11RHI* RHIDevice)
{
    UMotionBlurComponent* Comp = nullptr;
    if (Comp = Cast<UMotionBlurComponent>(M.SourceObject))
    {

    }
    else
    {
        return;
    }
    if (!IsApplicable(M)) return;

    // 1) 스왑 + SRV 언바인드 관리 (SceneColorSource 한 장만 읽음)
    FSwapGuard Swap(RHIDevice, /*FirstSlot*/0, /*NumSlotsToUnbind*/1);
    RHIDevice->OMSetDepthStencilState(EComparisonFunc::Always);
    RHIDevice->OMSetBlendState(false);
    ID3D11SamplerState* LinearClampSamplerState = RHIDevice->GetSamplerState(RHI_Sampler_Index::LinearClamp);
    ID3D11SamplerState* PointClampSamplerState = RHIDevice->GetSamplerState(RHI_Sampler_Index::PointClamp);
    ID3D11SamplerState* Smps[2] = { LinearClampSamplerState, PointClampSamplerState };
    RHIDevice->GetDeviceContext()->PSSetSamplers(0, 2, Smps);

    //CB
    FMatrix InvView = View->ViewMatrix.InverseAffine();
    FMatrix InvProjection = View->ProjectionMode == ECameraProjectionMode::Perspective ? View->ProjectionMatrix.InversePerspectiveProjection() : View->ProjectionMatrix.InverseOrthographicProjection();
    FMotionBlurCBuffer CBuffer;
    FMatrix CurVP = View->ViewMatrix * View->ProjectionMatrix;
    if (bFirstFrame)
    {
        bFirstFrame = false;
        LastFrameViewProj = CurVP;
    }
    CBuffer.CurInvVP = InvProjection * InvView;
    CBuffer.LastFrameVP = LastFrameViewProj;
    CBuffer.GaussianWeight = Comp->GaussianBlurWeight;
    CBuffer.MaxVelocity = Comp->MaxVelocity;
    CBuffer.SampleCount = Comp->SampleCount;
    LastFrameViewProj = CurVP;
    RHIDevice->SetAndUpdateConstantBuffer(CBuffer);

    //RenderTextures
    URenderTexture* VelocityRT = RHIDevice->GetRenderTexture("VelocityRT");
    VelocityRT->InitResolution(RHIDevice, 1.0f, DXGI_FORMAT_R16G16_UNORM);

    TArray<ID3D11ShaderResourceView*> SRVs;
    SRVs.Push(RHIDevice->GetSRV(RHI_SRV_Index::SceneColorSource));
    SRVs.Push(RHIDevice->GetSRV(RHI_SRV_Index::SceneDepth));
    Pass(RHIDevice, SRVs, VelocityRT->GetRTV(), MB_CalcScreenVelocityPSPath);
  

    SRVs.clear();
    SRVs.Push(RHIDevice->GetSRV(RHI_SRV_Index::SceneColorSource));
    SRVs.Push(VelocityRT->GetSRV());
    UShader* FullScreenTriangleVS = UResourceManager::GetInstance().Load<UShader>(UResourceManager::FullScreenVSPath);
    UShader* MotionBlurPS = UResourceManager::GetInstance().Load<UShader>(MB_MotionBlurPSPath);
    RHIDevice->PrepareShader(FullScreenTriangleVS, MotionBlurPS);
    RHIDevice->OMSetRenderTargets(ERTVMode::SceneColorTargetWithoutDepth);
    RHIDevice->GetDeviceContext()->PSSetShaderResources(0, 2, SRVs.data());
    RHIDevice->DrawFullScreenQuad();

    // 8) 확정
    Swap.Commit();

    ID3D11ShaderResourceView* NullSRVs[2] = { nullptr, nullptr };
    RHIDevice->GetDeviceContext()->PSSetShaderResources(0, 2, NullSRVs);
}
