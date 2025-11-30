#include "pch.h"
#include "DOFBlurPass.h"
#include "../SceneView.h"
#include "../../RHI/SwapGuard.h"
#include "../../RHI/ConstantBufferType.h"
#include "../../RHI/D3D11RHI.h"

void FDOFBlurPass::Execute(const FPostProcessModifier& M, FSceneView* View, D3D11RHI* RHIDevice)
{
    if (!IsApplicable(M)) return;

    // Viewport를 ViewRect/2 기준으로 설정 (게임 영역만)
    D3D11_VIEWPORT halfViewport;
    halfViewport.TopLeftX = View->ViewRect.MinX / 2.0f;
    halfViewport.TopLeftY = View->ViewRect.MinY / 2.0f;
    halfViewport.Width = View->ViewRect.Width() / 2.0f;
    halfViewport.Height = View->ViewRect.Height() / 2.0f;
    halfViewport.MinDepth = 0.0f;
    halfViewport.MaxDepth = 1.0f;
    RHIDevice->GetDeviceContext()->RSSetViewports(1, &halfViewport);

    // 셰이더 로드
    UShader* FullScreenVS = UResourceManager::GetInstance().Load<UShader>("Shaders/Utility/FullScreenTriangle_VS.hlsl");
    UShader* BlurPS = UResourceManager::GetInstance().Load<UShader>("Shaders/PostProcess/DOF_Blur_PS.hlsl");

    if (!FullScreenVS || !BlurPS || !FullScreenVS->GetVertexShader() || !BlurPS->GetPixelShader())
    {
        UE_LOG("DOFBlur: 셰이더 로드 실패!\n");
        return;
    }

    RHIDevice->PrepareShader(FullScreenVS, BlurPS);

    // Sampler
    ID3D11SamplerState* LinearClamp = RHIDevice->GetSamplerState(RHI_Sampler_Index::LinearClamp);
    ID3D11SamplerState* PointClamp = RHIDevice->GetSamplerState(RHI_Sampler_Index::PointClamp);
    if (!LinearClamp || !PointClamp)
    {
        UE_LOG("DOFBlur: Sampler 없음!\n");
        return;
    }
    ID3D11SamplerState* samplers[2] = { LinearClamp, PointClamp };
    RHIDevice->GetDeviceContext()->PSSetSamplers(0, 2, samplers);

    // Tile SRV 바인딩 (t1)
    if (TileSRV)
    {
        RHIDevice->GetDeviceContext()->PSSetShaderResources(1, 1, &TileSRV);
    }

    // Depth/Blend State
    RHIDevice->OMSetDepthStencilState(EComparisonFunc::Always);
    RHIDevice->OMSetBlendState(false);

    // Viewport Constants (b10)
    // ScreenSize = DOF 텍스처 크기 (SwapChain/2)
    // ViewportRect = 유효 렌더링 영역 (Viewport/2)
    UINT dofTexWidth = RHIDevice->GetSwapChainWidth() / 2;
    UINT dofTexHeight = RHIDevice->GetSwapChainHeight() / 2;

    FViewportConstants ViewportCB;
    ViewportCB.ViewportRect = FVector4(
        View->ViewRect.MinX / 2.0f,
        View->ViewRect.MinY / 2.0f,
        View->ViewRect.Width() / 2.0f,
        View->ViewRect.Height() / 2.0f
    );
    ViewportCB.ScreenSize = FVector4(
        static_cast<float>(dofTexWidth),
        static_cast<float>(dofTexHeight),
        1.0f / static_cast<float>(dofTexWidth),
        1.0f / static_cast<float>(dofTexHeight)
    );
    RHIDevice->SetAndUpdateConstantBuffer(ViewportCB);

    // Blur Radius (MaxBlurSize from DOF params)
    float NearBlurRadius = M.Payload.Params1.Z;  // MaxNearBlurSize
    float FarBlurRadius = M.Payload.Params1.W;   // MaxFarBlurSize

    ID3D11ShaderResourceView* NullSRV = nullptr;
    float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    // ===========================================
    // Ring-based 2D Blur (1패스로 원형 보케 생성)
    // ===========================================

    // 1. Far Field Blur
    {
        // SRV 언바인드
        RHIDevice->GetDeviceContext()->PSSetShaderResources(0, 1, &NullSRV);

        // RTV 설정: DOFTextures[2] (Temp - Far Field 블러 결과)
        ID3D11RenderTargetView* tempRTV = RHIDevice->GetDOFRTV(2);
        RHIDevice->OMSetCustomRenderTargets(1, &tempRTV, nullptr);
        RHIDevice->GetDeviceContext()->ClearRenderTargetView(tempRTV, clearColor);

        // SRV 바인딩: DOFTextures[0] (Far Field 원본)
        ID3D11ShaderResourceView* farSRV = RHIDevice->GetSRV(RHI_SRV_Index::DOFFar);
        RHIDevice->GetDeviceContext()->PSSetShaderResources(0, 1, &farSRV);

        // 상수 버퍼: Far Field
        FDOFBlurBufferType blurCB;
        blurCB.BlurDirection = FVector2D(0.0f, 0.0f);  // Ring은 방향 사용 안함
        blurCB.BlurRadius = FarBlurRadius;
        blurCB.IsFarField = 1;
        RHIDevice->SetAndUpdateConstantBuffer(blurCB);

        RHIDevice->DrawFullScreenQuad();
        RHIDevice->GetDeviceContext()->PSSetShaderResources(0, 1, &NullSRV);
    }

    // Far 결과를 DOFTextures[0]으로 복사 (Temp → Far)
    {
        ID3D11RenderTargetView* farRTV = RHIDevice->GetDOFRTV(0);
        RHIDevice->OMSetCustomRenderTargets(1, &farRTV, nullptr);

        ID3D11ShaderResourceView* tempSRV = RHIDevice->GetSRV(RHI_SRV_Index::DOFTempH);
        RHIDevice->GetDeviceContext()->PSSetShaderResources(0, 1, &tempSRV);

        // 단순 복사 (블러 없이)
        FDOFBlurBufferType blurCB;
        blurCB.BlurDirection = FVector2D(0.0f, 0.0f);
        blurCB.BlurRadius = 0.0f;  // 블러 없이 복사만
        blurCB.IsFarField = 1;
        RHIDevice->SetAndUpdateConstantBuffer(blurCB);

        RHIDevice->DrawFullScreenQuad();
        RHIDevice->GetDeviceContext()->PSSetShaderResources(0, 1, &NullSRV);
    }

    // 2. Near Field Blur
    {
        // RTV 설정: DOFTextures[2] (Temp - Near Field 블러 결과)
        ID3D11RenderTargetView* tempRTV = RHIDevice->GetDOFRTV(2);
        RHIDevice->OMSetCustomRenderTargets(1, &tempRTV, nullptr);
        RHIDevice->GetDeviceContext()->ClearRenderTargetView(tempRTV, clearColor);

        // SRV 바인딩: DOFTextures[1] (Near Field 원본)
        ID3D11ShaderResourceView* nearSRV = RHIDevice->GetSRV(RHI_SRV_Index::DOFNear);
        RHIDevice->GetDeviceContext()->PSSetShaderResources(0, 1, &nearSRV);

        // 상수 버퍼: Near Field
        FDOFBlurBufferType blurCB;
        blurCB.BlurDirection = FVector2D(0.0f, 0.0f);
        blurCB.BlurRadius = NearBlurRadius;
        blurCB.IsFarField = 0;
        RHIDevice->SetAndUpdateConstantBuffer(blurCB);

        RHIDevice->DrawFullScreenQuad();
        RHIDevice->GetDeviceContext()->PSSetShaderResources(0, 1, &NullSRV);
    }

    // Near 결과를 DOFTextures[1]으로 복사 (Temp → Near)
    {
        ID3D11RenderTargetView* nearRTV = RHIDevice->GetDOFRTV(1);
        RHIDevice->OMSetCustomRenderTargets(1, &nearRTV, nullptr);

        ID3D11ShaderResourceView* tempSRV = RHIDevice->GetSRV(RHI_SRV_Index::DOFTempH);
        RHIDevice->GetDeviceContext()->PSSetShaderResources(0, 1, &tempSRV);

        FDOFBlurBufferType blurCB;
        blurCB.BlurDirection = FVector2D(0.0f, 0.0f);
        blurCB.BlurRadius = 0.0f;
        blurCB.IsFarField = 0;
        RHIDevice->SetAndUpdateConstantBuffer(blurCB);

        RHIDevice->DrawFullScreenQuad();
        RHIDevice->GetDeviceContext()->PSSetShaderResources(0, 1, &NullSRV);
    }

    // Tile SRV 언바인드
    if (TileSRV)
    {
        RHIDevice->GetDeviceContext()->PSSetShaderResources(1, 1, &NullSRV);
    }

    // Viewport를 전체 해상도로 복원
    D3D11_VIEWPORT fullViewport;
    fullViewport.TopLeftX = 0.0f;
    fullViewport.TopLeftY = 0.0f;
    fullViewport.Width = static_cast<float>(RHIDevice->GetViewportWidth());
    fullViewport.Height = static_cast<float>(RHIDevice->GetViewportHeight());
    fullViewport.MinDepth = 0.0f;
    fullViewport.MaxDepth = 1.0f;
    RHIDevice->GetDeviceContext()->RSSetViewports(1, &fullViewport);

    // RTV 복원
    RHIDevice->OMSetRenderTargets(ERTVMode::SceneColorTargetWithoutDepth);
}
