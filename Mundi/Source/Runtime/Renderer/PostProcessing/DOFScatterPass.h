#pragma once
#include "PostProcessing.h"

// DOF Scatter Pass
// Near Field의 큰 CoC 픽셀을 Point Sprite로 확장하여 렌더링
// Gather로는 불가능한 진정한 "번짐" 효과 구현
class FDOFScatterPass
{
public:
    FDOFScatterPass() = default;
    ~FDOFScatterPass();

    void Initialize(D3D11RHI* RHIDevice, UINT Width, UINT Height);
    void Release();

    // Scatter 실행
    // Near Field 텍스처를 입력받아 Scatter 결과를 출력
    void Execute(D3D11RHI* RHIDevice, const FPostProcessModifier& M);

    // 결과 SRV
    ID3D11ShaderResourceView* GetScatterSRV() const { return ScatterSRV; }

    bool IsInitialized() const { return bInitialized; }

private:
    bool bInitialized = false;

    UINT Width = 0;
    UINT Height = 0;

    // Scatter 출력 텍스처 (Near Field와 같은 크기)
    ID3D11Texture2D* ScatterTexture = nullptr;
    ID3D11RenderTargetView* ScatterRTV = nullptr;
    ID3D11ShaderResourceView* ScatterSRV = nullptr;

    // Constant Buffer
    ID3D11Buffer* ScatterCB = nullptr;

    // Additive Blend State
    ID3D11BlendState* AdditiveBlendState = nullptr;

    void CreateResources(D3D11RHI* RHIDevice);
    void ReleaseResources();
};
