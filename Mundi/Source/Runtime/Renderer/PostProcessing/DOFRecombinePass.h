#pragma once
#include "PostProcessing.h"

class FDOFRecombinePass final : public IPostProcessPass
{
public:
    virtual void Execute(const FPostProcessModifier& M, FSceneView* View, D3D11RHI* RHIDevice) override;

    // Scatter SRV 설정 (Near Field Scatter 결과)
    void SetScatterSRV(ID3D11ShaderResourceView* InScatterSRV) { ScatterSRV = InScatterSRV; }

private:
    ID3D11ShaderResourceView* ScatterSRV = nullptr;
};
