#pragma once
#include "CameraModifierBase.h"
#include "PostProcessing/PostProcessing.h"

class UCamMod_MotionBlur : public UCameraModifierBase
{

public:
    DECLARE_CLASS(UCamMod_MotionBlur, UCameraModifierBase)

    UCamMod_MotionBlur() = default;
    virtual ~UCamMod_MotionBlur() = default;

    float CenterX = 0.5f;        // 블러 중심점 X (0~1)
    float CenterY = 0.5f;        // 블러 중심점 Y (0~1)
    float Intensity = 1.0f;      // 블러 강도 (0~1)
    int32 SampleCount = 16;      // 샘플 개수 (8~32)

    virtual void ApplyToView(float DeltaTime, FMinimalViewInfo* ViewInfo) override {};

    virtual void CollectPostProcess(TArray<FPostProcessModifier>& Out) override
    {
        if (!bEnabled) return;

        FPostProcessModifier M;
        M.Type = EPostProcessEffectType::MotionBlur;
        M.Priority = Priority;
        M.bEnabled = true;
        M.Weight = Weight;
        M.SourceObject = this;

        // Payload.Params0: (CenterX, CenterY, Intensity, SampleCount)
        M.Payload.Params0 = FVector4(CenterX, CenterY, Intensity, static_cast<float>(SampleCount));

        Out.Add(M);
    }
};
