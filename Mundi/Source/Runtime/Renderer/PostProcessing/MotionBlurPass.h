#pragma once
#include "PostProcessing.h"

class FMotionBlurPass final : public IPostProcessPass
{
public:
    static const char* MB_CalcScreenVelocityPSPath;
    static const char* MB_MotionBlurPSPath;

    virtual void Execute(const FPostProcessModifier& M, FSceneView* View, D3D11RHI* RHIDevice) override;
private:
    //매프레임 재생성되는 패스라서 어쩔수없이 static으로 변수활용
    static bool bFirstFrame;
    static FMatrix LastFrameViewProj;
};
