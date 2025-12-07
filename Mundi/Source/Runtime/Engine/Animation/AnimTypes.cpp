#include "pch.h"
#include "AnimDateModel.h"

void IAnimPoseProvider::IgnoreRootBoneTransform(FTransform* OutRootPose, const FName& InRootBoneName, const UAnimDataModel* InDataModel)
{
	/*FTransform RootTransform = *OutRootPose;
	RootTransform.Translation = InDataModel->EvaluateBoneTrackTransform(InRootBoneName, 0.0f, true).Translation;
	*OutRootPose = RootTransform;*/
	OutRootPose->Translation = FVector(0.0f, 0.0f, 0.0f);
	OutRootPose->Rotation = FQuat::Identity();
}
