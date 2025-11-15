#include "pch.h"
#include "AnimSequence.h"
#include "AnimDateModel.h"

IMPLEMENT_CLASS(UAnimSequence)

UAnimSequence::UAnimSequence()
{
}

float UAnimSequence::GetPlayLength() const
{
    const UAnimDataModel* Model = GetDataModel();
    if (Model)
    {
        return Model->GetPlayLength();
    }
    return 0.0f;
}

void UAnimSequence::GetAnimationPose(FPoseContext& OutPoseContext, const FAnimExtractContext& ExtractionContext)
{
    GetBonePose(OutPoseContext, ExtractionContext);
}

void UAnimSequence::GetBonePose(FPoseContext& OutPoseContext, const FAnimExtractContext& ExtractionContext)
{
    const UAnimDataModel* Model = GetDataModel();
    if (!Model)
    {
        return;
    }

    const int32 FrameRate = Model->GetFrameRate();
    const int32 NumberOfFrames = Model->GetNumberOfFrames();

    if (NumberOfFrames <= 0 || FrameRate <= 0)
    {
        return;
    }

    // Get current time and convert to frame
    float CurrentTime = static_cast<float>(ExtractionContext.CurrentTime);

    // Handle looping
    if (ExtractionContext.bLooping)
    {
        float PlayLength = Model->GetPlayLength();
        if (PlayLength > 0.0f)
        {
            CurrentTime = FMath::Fmod(CurrentTime, PlayLength);
            if (CurrentTime < 0.0f)
            {
                CurrentTime += PlayLength;
            }
        }
    }

    // Get all bone tracks and evaluate them
    const TArray<FBoneAnimationTrack>& BoneTracks = Model->GetBoneAnimationTracks();

    for (int32 TrackIndex = 0; TrackIndex < BoneTracks.Num(); ++TrackIndex)
    {
        const FBoneAnimationTrack& Track = BoneTracks[TrackIndex];
        FName BoneName = Track.Name;

        // Evaluate bone transform at current time
        FTransform BoneTransform = Model->EvaluateBoneTrackTransform(BoneName, CurrentTime, true);

        // Store in pose context (you'll need to map bone name to index)
        // For now, we'll just use track index
        if (TrackIndex < OutPoseContext.Pose.Num())
        {
            OutPoseContext.Pose[TrackIndex] = BoneTransform;
        }
    }
}