#include "pch.h"
#include "AnimSingleNodeInstance.h"
#include "AnimSequence.h"

void UAnimSingleNodeInstance::PlaySingleNode(UAnimSequence* Sequence, bool bLoop, float InPlayRate)
{
    if (!Sequence)
    {
        UE_LOG("UAnimSingleNodeInstance::PlaySingleNode - Invalid sequence");
        StopSingleNode();
        return;
    }

    SingleNodeSequence = Sequence;
    PlaySequence(Sequence, bLoop, InPlayRate);
}

void UAnimSingleNodeInstance::StopSingleNode()
{
    SingleNodeSequence = nullptr;
    StopSequence();
}

void UAnimSingleNodeInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    // SingleNodeSequence가 없으면 부모의 PoseProvider 체크로 넘김
    if (!SingleNodeSequence)
    {
        // 부모 클래스에서 PoseProvider가 있을 수 있으므로 호출
        UAnimInstance::NativeUpdateAnimation(DeltaSeconds);
        return;
    }

    UAnimInstance::NativeUpdateAnimation(DeltaSeconds);
}

