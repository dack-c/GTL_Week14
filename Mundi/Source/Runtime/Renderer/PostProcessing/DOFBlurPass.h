#pragma once
#include "PostProcessing.h"

class FDOFBlurPass final : public IPostProcessPass
{
public:
    virtual void Execute(const FPostProcessModifier& M, FSceneView* View, D3D11RHI* RHIDevice) override;

    // Tile SRV 설정 (SceneRenderer에서 호출)
    void SetTileSRV(ID3D11ShaderResourceView* InTileSRV) { TileSRV = InTileSRV; }

private:
    ID3D11ShaderResourceView* TileSRV = nullptr;
};
