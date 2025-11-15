#pragma once
#include "AnimSequenceBase.h"

// Forward declarations
struct FPoseContext
{
    TArray<FTransform> Pose;

    FPoseContext() {}
    explicit FPoseContext(int32 NumBones)
    {
        Pose.SetNum(NumBones);
    }
};

class UAnimSequence : public UAnimSequenceBase
{
    DECLARE_CLASS(UAnimSequence, UAnimSequenceBase)

public:
    UAnimSequence();
    virtual ~UAnimSequence() = default;

    // Get animation pose at specific time
    void GetAnimationPose(FPoseContext& OutPoseContext, const FAnimExtractContext& ExtractionContext);

    // Get bone pose for specific bones
    void GetBonePose(FPoseContext& OutPoseContext, const FAnimExtractContext& ExtractionContext);

    // Override GetPlayLength from base class
    virtual float GetPlayLength() const override;
};
