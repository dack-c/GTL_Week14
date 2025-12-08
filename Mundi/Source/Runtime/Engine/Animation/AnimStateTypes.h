#pragma once
#include "pch.h"
#include "AnimSequence.h"
#include "AnimTypes.h"

/**
 * @brief 애니메이션 상태 정의
 * @note IAnimPoseProvider를 지원하여 AnimSequence, BlendSpace 등 다양한 애니메이션 소스 사용 가능
 */
struct FAnimationState
{
    FName Name;                     // 상태 이름 (예: "Idle", "Walk", "Run")
    UAnimSequence* Sequence;        // 재생할 시퀀스 (기존 호환용)
    IAnimPoseProvider* PoseProvider; // 포즈 제공자 (BlendSpace 등)
    bool bLoop;                     // 루프 여부
    float PlayRate;                 // 재생 속도

    std::function<void()> OnUpdate; // 매 프레임 호출될 업데이트 함수 (EvaluatePin 방식)

    FAnimationState()
        : Name("None")
        , Sequence(nullptr)
        , PoseProvider(nullptr)
        , bLoop(true)
        , PlayRate(1.0f)
    {
    }

    // 기존 AnimSequence 생성자 (호환성 유지)
    FAnimationState(const FName& InName, UAnimSequence* InSequence, bool InLoop = true, float InPlayRate = 1.0f)
        : Name(InName)
        , Sequence(InSequence)
        , PoseProvider(InSequence)  // AnimSequence는 IAnimPoseProvider를 구현
        , bLoop(InLoop)
        , PlayRate(InPlayRate)
    {
    }

    // PoseProvider 직접 설정 생성자 (BlendSpace 등)
    FAnimationState(const FName& InName, IAnimPoseProvider* InPoseProvider, bool InLoop = true, float InPlayRate = 1.0f)
        : Name(InName)
        , Sequence(nullptr)
        , PoseProvider(InPoseProvider)
        , bLoop(InLoop)
        , PlayRate(InPlayRate)
    {
    }
};

/**
 * @brief 상태 전이 조건
 */
struct FStateTransition
{
    FName FromState;            // 출발 상태
    FName ToState;              // 도착 상태
    std::function<bool()> Condition;  // 전이 조건 함수
    float BlendTime;            // 블렌드 시간

    FStateTransition()
        : FromState("None")
        , ToState("None")
        , Condition(nullptr)
        , BlendTime(0.2f)
    {
    }

    FStateTransition(const FName& From, const FName& To, std::function<bool()> InCondition, float InBlendTime = 0.2f)
        : FromState(From)
        , ToState(To)
        , Condition(InCondition)
        , BlendTime(InBlendTime)
    {
    }
};

/**
 * @brief 애니메이션 재생 상태를 관리하는 구조체
 * @note 포즈 제공자 기반으로 리팩터링됨 - AnimSequence, BlendSpace 등 모두 지원
 */
struct FAnimationPlayState
{
    /** 포즈를 제공하는 소스 (AnimSequence, BlendSpace1D 등) */
    IAnimPoseProvider* PoseProvider = nullptr;

    /** 기존 호환성을 위한 AnimSequence 참조 (노티파이 등에서 사용) */
    UAnimSequence* Sequence = nullptr;

    /** BlendSpace용 파라미터 값 */
    float BlendParameter = 0.0f;

    float CurrentTime = 0.0f;
    float PlayRate = 1.0f;
    float BlendWeight = 1.0f;
    bool bIsLooping = false;
    bool bIsPlaying = false;
    uint32 loopCount = 1; // 몇 번째 루프인지
};