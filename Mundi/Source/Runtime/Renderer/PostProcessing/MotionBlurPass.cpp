#include "pch.h"
#include "MotionBlurPass.h"

#include "SwapGuard.h"

void FMotionBlurPass::Execute(const FPostProcessModifier& M, FSceneView* View, D3D11RHI* RHIDevice)
{
    if (!IsApplicable(M)) return;

    // 1) 스왑 + SRV 언바인드 관리
    FSwapGuard Swap(RHIDevice, /*FirstSlot*/0, /*NumSlotsToUnbind*/1);

    // 2) 타깃 RTV 설정
    RHIDevice->OMSetRenderTargets(ERTVMode::SceneColorTargetWithoutDepth);

    // Depth State: Depth Test/Write 모두 OFF
    RHIDevice->OMSetDepthStencilState(EComparisonFunc::Always);
    RHIDevice->OMSetBlendState(false);

    // 3) 셰이더 로드
    UShader* FullScreenTriangleVS = UResourceManager::GetInstance().Load<UShader>("Shaders/Utility/FullScreenTriangle_VS.hlsl");
    UShader* MotionBlurPS = UResourceManager::GetInstance().Load<UShader>("Shaders/PostProcess/MotionBlur_PS.hlsl");
    if (!FullScreenTriangleVS || !FullScreenTriangleVS->GetVertexShader() || !MotionBlurPS || !MotionBlurPS->GetPixelShader())
    {
        UE_LOG("MotionBlur: 셰이더 로드 실패!\n");
        return;
    }

    RHIDevice->PrepareShader(FullScreenTriangleVS, MotionBlurPS);

    // 4) SRV / Sampler
    ID3D11ShaderResourceView* SceneSRV = RHIDevice->GetSRV(RHI_SRV_Index::SceneColorSource);
    ID3D11SamplerState* LinearClampSamplerState = RHIDevice->GetSamplerState(RHI_Sampler_Index::LinearClamp);

    if (!SceneSRV || !LinearClampSamplerState)
    {
        UE_LOG("MotionBlur: Scene SRV / Sampler is null!\n");
        return;
    }

    RHIDevice->GetDeviceContext()->PSSetShaderResources(0, 1, &SceneSRV);
    RHIDevice->GetDeviceContext()->PSSetSamplers(0, 1, &LinearClampSamplerState);

    // 5) 상수 버퍼 업데이트
    FMotionBlurBufferType MotionBlurConstant;
    const FViewportRect& ViewportRect = View->ViewRect;
    const float CenterX = static_cast<float>(ViewportRect.MinX + ViewportRect.MaxX) * 0.5f;
    const float CenterY = static_cast<float>(ViewportRect.MinY + ViewportRect.MaxY) * 0.5f;
    
    const UINT SwapChainWidth = RHIDevice->GetSwapChainWidth();
    const UINT SwapChainHeight = RHIDevice->GetSwapChainHeight();
    
    MotionBlurConstant.Center = FVector2D(
         CenterX / static_cast<float>(SwapChainWidth),
         CenterY / static_cast<float>(SwapChainHeight)
    );
    MotionBlurConstant.Intensity = M.Payload.Params0.Z;  // Intensity (0~1)
    MotionBlurConstant.SampleCount = static_cast<int32>(M.Payload.Params0.W);  // Sample Count (기본 16)
    MotionBlurConstant.Weight = M.Weight;

    RHIDevice->SetAndUpdateConstantBuffer(MotionBlurConstant);

    // 6) Draw
    RHIDevice->DrawFullScreenQuad();

    // 7) 확정
    Swap.Commit();
}
