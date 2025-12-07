#include "pch.h"
#include "AnimDateModel.h"

void IAnimPoseProvider::IgnoreRootBoneTransform(FTransform* OutRootPose, const FName& InRootBoneName, const UAnimDataModel* InDataModel)
{
	const FTransform FirstFrameTransform = InDataModel->EvaluateBoneTrackTransform(InRootBoneName, 0.0f, true);
	OutRootPose->Translation = FirstFrameTransform.Translation;
}
