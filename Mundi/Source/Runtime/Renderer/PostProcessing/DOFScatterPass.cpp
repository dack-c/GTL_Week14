#include "pch.h"
#include "DOFScatterPass.h"
#include "../../RHI/D3D11RHI.h"
#include "../Shader.h"

// Scatter Constant Buffer 구조체
struct FScatterConstants
{
    FVector2D TexelSize;      // 1/Width, 1/Height
    FVector2D ScreenSize;     // Width, Height
    float MaxBlurRadius;      // 최대 블러 반경 (pixels)
    float CocThreshold;       // Scatter 임계값
    float BokehIntensity;     // 보케 밝기
    UINT PixelOffset;         // 배치 시작 픽셀 인덱스
};

FDOFScatterPass::~FDOFScatterPass()
{
    Release();
}

void FDOFScatterPass::Initialize(D3D11RHI* RHIDevice, UINT InWidth, UINT InHeight)
{
    if (bInitialized)
    {
        if (Width != InWidth || Height != InHeight)
        {
            Release();
        }
        else
        {
            return;
        }
    }

    Width = InWidth;
    Height = InHeight;

    CreateResources(RHIDevice);
    bInitialized = true;

    UE_LOG("DOFScatterPass: Initialized (%dx%d)\n", Width, Height);
}

void FDOFScatterPass::Release()
{
    ReleaseResources();
    bInitialized = false;
}

void FDOFScatterPass::CreateResources(D3D11RHI* RHIDevice)
{
    auto* Device = RHIDevice->GetDevice();

    // Scatter 출력 텍스처 (RGBA16F for HDR)
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = Width;
    texDesc.Height = Height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    HRESULT hr = Device->CreateTexture2D(&texDesc, nullptr, &ScatterTexture);
    if (FAILED(hr))
    {
        UE_LOG("[error] DOFScatterPass: Failed to create scatter texture\n");
        return;
    }

    // RTV
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = texDesc.Format;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;

    hr = Device->CreateRenderTargetView(ScatterTexture, &rtvDesc, &ScatterRTV);
    if (FAILED(hr))
    {
        UE_LOG("[error] DOFScatterPass: Failed to create scatter RTV\n");
        return;
    }

    // SRV
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;

    hr = Device->CreateShaderResourceView(ScatterTexture, &srvDesc, &ScatterSRV);
    if (FAILED(hr))
    {
        UE_LOG("[error] DOFScatterPass: Failed to create scatter SRV\n");
        return;
    }

    // Constant Buffer
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(FScatterConstants);
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = Device->CreateBuffer(&cbDesc, nullptr, &ScatterCB);
    if (FAILED(hr))
    {
        UE_LOG("[error] DOFScatterPass: Failed to create constant buffer\n");
        return;
    }

    // Additive Blend State
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    hr = Device->CreateBlendState(&blendDesc, &AdditiveBlendState);
    if (FAILED(hr))
    {
        UE_LOG("[error] DOFScatterPass: Failed to create additive blend state\n");
    }
}

void FDOFScatterPass::ReleaseResources()
{
    if (ScatterSRV) { ScatterSRV->Release(); ScatterSRV = nullptr; }
    if (ScatterRTV) { ScatterRTV->Release(); ScatterRTV = nullptr; }
    if (ScatterTexture) { ScatterTexture->Release(); ScatterTexture = nullptr; }
    if (ScatterCB) { ScatterCB->Release(); ScatterCB = nullptr; }
    if (AdditiveBlendState) { AdditiveBlendState->Release(); AdditiveBlendState = nullptr; }
}

void FDOFScatterPass::Execute(D3D11RHI* RHIDevice, const FPostProcessModifier& M)
{
    // DOF 텍스처 크기 (1/2 해상도)
    UINT dofWidth = RHIDevice->GetSwapChainWidth() / 2;
    UINT dofHeight = RHIDevice->GetSwapChainHeight() / 2;

    Initialize(RHIDevice, dofWidth, dofHeight);

    if (!bInitialized)
    {
        return;
    }

    auto* DeviceContext = RHIDevice->GetDeviceContext();

    // 셰이더 로드
    UShader* ScatterVS = UResourceManager::GetInstance().Load<UShader>("Shaders/PostProcess/DOF_Scatter_VS.hlsl");
    UShader* ScatterPS = UResourceManager::GetInstance().Load<UShader>("Shaders/PostProcess/DOF_Scatter_PS.hlsl");

    if (!ScatterVS || !ScatterPS || !ScatterVS->GetVertexShader() || !ScatterPS->GetPixelShader())
    {
        UE_LOG("[error] DOFScatterPass: Failed to load shaders\n");
        return;
    }

    // Near Field SRV 가져오기
    ID3D11ShaderResourceView* NearFieldSRV = RHIDevice->GetDOFSRV(1);
    if (!NearFieldSRV)
    {
        return;
    }

    // Constant Buffer 기본값 (PixelOffset은 배치마다 업데이트)
    float maxBlurRadius = M.Payload.Params1.Z;  // MaxNearBlurSize

    // Viewport 설정 (Half Res)
    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = (float)Width;
    viewport.Height = (float)Height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    DeviceContext->RSSetViewports(1, &viewport);

    // RTV 클리어 및 설정
    float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    DeviceContext->ClearRenderTargetView(ScatterRTV, clearColor);
    DeviceContext->OMSetRenderTargets(1, &ScatterRTV, nullptr);

    // Additive Blending
    float blendFactor[4] = { 0, 0, 0, 0 };
    DeviceContext->OMSetBlendState(AdditiveBlendState, blendFactor, 0xFFFFFFFF);

    // Depth/Stencil 비활성화
    RHIDevice->OMSetDepthStencilState(EComparisonFunc::Always);

    // 셰이더 설정
    DeviceContext->VSSetShader(ScatterVS->GetVertexShader(), nullptr, 0);
    DeviceContext->PSSetShader(ScatterPS->GetPixelShader(), nullptr, 0);
    DeviceContext->IASetInputLayout(nullptr);  // SV_VertexID/InstanceID만 사용

    // 리소스 바인딩
    DeviceContext->VSSetShaderResources(0, 1, &NearFieldSRV);
    DeviceContext->VSSetConstantBuffers(0, 1, &ScatterCB);

    // Primitive Topology: Triangle Strip (쿼드 렌더링)
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    // 모든 픽셀에 대해 쿼드 그리기
    // 각 픽셀 = 4 정점 (Triangle Strip)
    UINT totalPixels = Width * Height;
    UINT verticesPerQuad = 4;

    // 한 번에 너무 많이 그리면 GPU 멈춤 → 배치 처리
    UINT MAX_QUADS_PER_BATCH = 65536;  // 64K 쿼드씩

    for (UINT startPixel = 0; startPixel < totalPixels; startPixel += MAX_QUADS_PER_BATCH)
    {
        UINT remaining = totalPixels - startPixel;
        UINT quadCount = (remaining < MAX_QUADS_PER_BATCH) ? remaining : MAX_QUADS_PER_BATCH;

        // 배치마다 Constant Buffer 업데이트 (PixelOffset 포함)
        {
            D3D11_MAPPED_SUBRESOURCE mapped;
            DeviceContext->Map(ScatterCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

            FScatterConstants* cb = (FScatterConstants*)mapped.pData;
            cb->TexelSize = FVector2D(1.0f / Width, 1.0f / Height);
            cb->ScreenSize = FVector2D((float)Width, (float)Height);
            cb->MaxBlurRadius = maxBlurRadius * 0.5f;
            cb->CocThreshold = 0.15f;
            cb->BokehIntensity = 1.0f;
            cb->PixelOffset = startPixel;  // 배치 시작 오프셋

            DeviceContext->Unmap(ScatterCB, 0);
        }

        // DrawInstanced: 각 인스턴스 = 1 쿼드(4 정점)
        // SV_InstanceID는 항상 0부터 시작하므로, PixelOffset으로 보정
        DeviceContext->DrawInstanced(verticesPerQuad, quadCount, 0, 0);
    }

    // 리소스 언바인드
    ID3D11ShaderResourceView* nullSRV = nullptr;
    DeviceContext->VSSetShaderResources(0, 1, &nullSRV);

    // RTV 언바인드
    ID3D11RenderTargetView* nullRTV = nullptr;
    DeviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);

    // Blend State 복원
    RHIDevice->OMSetBlendState(false);

    // Primitive Topology 복원
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Viewport 복원 (전체 해상도 = Half Res * 2)
    D3D11_VIEWPORT fullViewport;
    fullViewport.TopLeftX = 0.0f;
    fullViewport.TopLeftY = 0.0f;
    fullViewport.Width = static_cast<float>(Width * 2);
    fullViewport.Height = static_cast<float>(Height * 2);
    fullViewport.MinDepth = 0.0f;
    fullViewport.MaxDepth = 1.0f;
    DeviceContext->RSSetViewports(1, &fullViewport);
}
