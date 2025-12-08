#include "pch.h"
#include "AnimTypes.h"
#include "AnimDateModel.h"

void IAnimPoseProvider::IgnoreRootBoneTransform(FTransform* OutRootPose, const FName& InRootBoneName, const UAnimDataModel* InDataModel)
{
	const FTransform FirstFrameTransform = InDataModel->EvaluateBoneTrackTransform(InRootBoneName, 0.0f, true);
	OutRootPose->Translation = FirstFrameTransform.Translation;
}

FAnimationState::FAnimationState()
    : Name("None")
    , Sequence(nullptr)
    , PoseProvider(nullptr)
    , bLoop(true)
    , PlayRate(1.0f)
{
}

FAnimationState::FAnimationState(const FName& InName, UAnimSequence* InSequence, bool InLoop, float InPlayRate)
    : Name(InName)
    , Sequence(InSequence)
    , PoseProvider(InSequence)  // InSequence is a UAnimSequence*, which implements IAnimPoseProvider
    , bLoop(InLoop)
    , PlayRate(InPlayRate)
{
}

FAnimationState::FAnimationState(const FName& InName, IAnimPoseProvider* InPoseProvider, bool InLoop, float InPlayRate)
    : Name(InName)
    , Sequence(nullptr) // Or derive from PoseProvider if possible/needed
    , PoseProvider(InPoseProvider)
    , bLoop(InLoop)
    , PlayRate(InPlayRate)
{
}

FStateTransition::FStateTransition()
    : FromState("None")
    , ToState("None")
    , Condition(nullptr)
    , BlendTime(0.2f)
{
}

FStateTransition::FStateTransition(const FName& From, const FName& To, std::function<bool()> InCondition, float InBlendTime)
    : FromState(From)
    , ToState(To)
    , Condition(InCondition)
    , BlendTime(InBlendTime)
{
}