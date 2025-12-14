#include "pch.h"
#include "K2Node_CharacterMovement.h"

#include "BlueprintActionDatabase.h"
#include "CharacterMovementComponent.h"
#include "SkeletalMeshComponent.h"
#include "Source/Runtime/Engine/Animation/AnimInstance.h"
#include "Source/Runtime/Engine/Animation/AnimationStateMachine.h"
#include "ImGui/imgui_stdlib.h"
#include "Character.h"
#include "CapsuleComponent.h"
#include "Source/Runtime/Engine/Physics/PhysScene.h"
#include "AABB.h"

// ----------------------------------------------------------------
//	Internal Helper: 컨텍스트에서 MovementComponent 탐색
// ----------------------------------------------------------------
static UCharacterMovementComponent* GetMovementFromContext(FBlueprintContext* Context)
{
    if (!Context || !Context->SourceObject)
    {
        return nullptr;
    }

    UAnimInstance* AnimInstance = Cast<UAnimInstance>(Context->SourceObject);
    if (!AnimInstance)
    {
        return nullptr;
    }

    USkeletalMeshComponent* MeshComp = AnimInstance->GetOwningComponent();
    if (!MeshComp)
    {
        return nullptr;
    }

    auto* OwnerActor = MeshComp->GetOwner();
    if (!OwnerActor)
    {
        return nullptr;
    }

    UCharacterMovementComponent* MoveComp = Cast<UCharacterMovementComponent>(

        OwnerActor->GetComponent(UCharacterMovementComponent::StaticClass())
    );

    return MoveComp;
}

// ----------------------------------------------------------------
//	[GetIsFalling] 
// ----------------------------------------------------------------

IMPLEMENT_CLASS(UK2Node_GetIsFalling)

UK2Node_GetIsFalling::UK2Node_GetIsFalling()
{
    TitleColor = ImColor(100, 200, 100); // Pure Node Green
}

void UK2Node_GetIsFalling::AllocateDefaultPins()
{
    CreatePin(EEdGraphPinDirection::EGPD_Output, FEdGraphPinCategory::Bool, "Is Falling");
}

FBlueprintValue UK2Node_GetIsFalling::EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context)
{
    UCharacterMovementComponent* MoveComp = GetMovementFromContext(Context);

    if (OutputPin->PinName == "Is Falling")
    {
        if (!MoveComp)
        {
            return FBlueprintValue(false); 
        }
        return FBlueprintValue(MoveComp->IsFalling());
    }

    return FBlueprintValue{};
}

void UK2Node_GetIsFalling::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(GetClass());
    Spawner->MenuName = GetNodeTitle();
    Spawner->Category = GetMenuCategory();
    ActionRegistrar.AddAction(Spawner);
}

// ----------------------------------------------------------------
//	[GetIsSliding] 
// ----------------------------------------------------------------

IMPLEMENT_CLASS(UK2Node_GetIsSliding)

UK2Node_GetIsSliding::UK2Node_GetIsSliding()
{
    TitleColor = ImColor(100, 200, 100); // Pure Node Green
}

void UK2Node_GetIsSliding::AllocateDefaultPins()
{
    CreatePin(EEdGraphPinDirection::EGPD_Output, FEdGraphPinCategory::Bool, "Is Sliding");
}

FBlueprintValue UK2Node_GetIsSliding::EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context)
{
    UCharacterMovementComponent* MoveComp = GetMovementFromContext(Context);

    if (OutputPin->PinName == "Is Sliding")
    {
        if (!MoveComp)
        {
            return FBlueprintValue(false);
        }
        return FBlueprintValue(MoveComp->IsSliding());
    }

    return FBlueprintValue{};
}

void UK2Node_GetIsSliding::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(GetClass());
    Spawner->MenuName = GetNodeTitle();
    Spawner->Category = GetMenuCategory();
    ActionRegistrar.AddAction(Spawner);
}

// ----------------------------------------------------------------
//	[GetIsJumping] 
// ----------------------------------------------------------------

IMPLEMENT_CLASS(UK2Node_GetIsJumping)

UK2Node_GetIsJumping::UK2Node_GetIsJumping()
{
    TitleColor = ImColor(100, 200, 100); // Pure Node Green
}

void UK2Node_GetIsJumping::AllocateDefaultPins()
{
    CreatePin(EEdGraphPinDirection::EGPD_Output, FEdGraphPinCategory::Bool, "Is Jumping");
}

FBlueprintValue UK2Node_GetIsJumping::EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context)
{
    UCharacterMovementComponent* MoveComp = GetMovementFromContext(Context);

    if (OutputPin->PinName == "Is Jumping")
    {
        if (!MoveComp)
        {
            return FBlueprintValue(false);
        }
        return FBlueprintValue(MoveComp->IsJumping());
    }

    return FBlueprintValue{};
}

void UK2Node_GetIsJumping::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(GetClass());
    Spawner->MenuName = GetNodeTitle();
    Spawner->Category = GetMenuCategory();
    ActionRegistrar.AddAction(Spawner);
}

// ----------------------------------------------------------------
//	[GetNeedRolling] 
// ----------------------------------------------------------------

IMPLEMENT_CLASS(UK2Node_GetNeedRolling)

UK2Node_GetNeedRolling::UK2Node_GetNeedRolling()
{
    TitleColor = ImColor(100, 200, 100); // Pure Node Green
}

void UK2Node_GetNeedRolling::AllocateDefaultPins()
{
    CreatePin(EEdGraphPinDirection::EGPD_Output, FEdGraphPinCategory::Bool, "Need Rolling");
}

FBlueprintValue UK2Node_GetNeedRolling::EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context)
{
    UCharacterMovementComponent* MoveComp = GetMovementFromContext(Context);

    if (OutputPin->PinName == "Need Rolling")
    {
        if (!MoveComp)
        {
            return FBlueprintValue(false);
        }
        return FBlueprintValue(MoveComp->NeedRolling());
    }

    return FBlueprintValue{};
}

void UK2Node_GetNeedRolling::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(GetClass());
    Spawner->MenuName = GetNodeTitle();
    Spawner->Category = GetMenuCategory();
    ActionRegistrar.AddAction(Spawner);
}

// ----------------------------------------------------------------
//	[GetAirTime] 
// ----------------------------------------------------------------

IMPLEMENT_CLASS(UK2Node_GetAirTime)

UK2Node_GetAirTime::UK2Node_GetAirTime()
{
    TitleColor = ImColor(100, 200, 100);
}

void UK2Node_GetAirTime::AllocateDefaultPins()
{
    CreatePin(EEdGraphPinDirection::EGPD_Output, FEdGraphPinCategory::Float, "AirTime");
}

FBlueprintValue UK2Node_GetAirTime::EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context)
{
    auto* MoveComp = GetMovementFromContext(Context);

    if (!MoveComp)
    {
        return FBlueprintValue(0.0f);
    }

    float AirTime = MoveComp->GetAirTime();

    if (OutputPin->PinName == "AirTime")
    {
        return FBlueprintValue(AirTime);
    }

    return FBlueprintValue(0.0f);
}

void UK2Node_GetAirTime::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(GetClass());
    Spawner->MenuName = GetNodeTitle();
    Spawner->Category = GetMenuCategory();
    ActionRegistrar.AddAction(Spawner);
}

// ----------------------------------------------------------------
//	[GetVelocity] 
// ----------------------------------------------------------------

IMPLEMENT_CLASS(UK2Node_GetVelocity)

UK2Node_GetVelocity::UK2Node_GetVelocity()
{
    TitleColor = ImColor(100, 200, 100);
}

void UK2Node_GetVelocity::AllocateDefaultPins()
{
    CreatePin(EEdGraphPinDirection::EGPD_Output, FEdGraphPinCategory::Float, "X");
    CreatePin(EEdGraphPinDirection::EGPD_Output, FEdGraphPinCategory::Float, "Y");
    CreatePin(EEdGraphPinDirection::EGPD_Output, FEdGraphPinCategory::Float, "Z");
}

FBlueprintValue UK2Node_GetVelocity::EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context)
{
    auto* MoveComp = GetMovementFromContext(Context);

    if (!MoveComp)
    {
        return FBlueprintValue(0.0f);
    }

    FVector Velocity = MoveComp->GetVelocity();

    if (OutputPin->PinName == "X")
    {
        return FBlueprintValue(Velocity.X);
    }
    else if (OutputPin->PinName == "Y")
    {
        return FBlueprintValue(Velocity.Y);
    }
    else if (OutputPin->PinName == "Z")
    {
        return FBlueprintValue(Velocity.Z);
    }

    return FBlueprintValue(0.0f);
}

void UK2Node_GetVelocity::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(GetClass());
    Spawner->MenuName = GetNodeTitle();
    Spawner->Category = GetMenuCategory();
    ActionRegistrar.AddAction(Spawner);
}

// ----------------------------------------------------------------
//	[GetLocalVelocity]
/// ----------------------------------------------------------------

IMPLEMENT_CLASS(UK2Node_GetLocalVelocity)

UK2Node_GetLocalVelocity::UK2Node_GetLocalVelocity()
{
    TitleColor = ImColor(100, 200, 100);
}

void UK2Node_GetLocalVelocity::AllocateDefaultPins()
{
    // X: 좌우 (Right 방향 내적), Y: 앞뒤 (Forward 방향 내적), Z: 위아래 (Up 방향 내적)
    CreatePin(EEdGraphPinDirection::EGPD_Output, FEdGraphPinCategory::Float, "X");
    CreatePin(EEdGraphPinDirection::EGPD_Output, FEdGraphPinCategory::Float, "Y");
    CreatePin(EEdGraphPinDirection::EGPD_Output, FEdGraphPinCategory::Float, "Z");
}

FBlueprintValue UK2Node_GetLocalVelocity::EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context)
{
    auto* MoveComp = GetMovementFromContext(Context);

    if (!MoveComp)
    {
        return FBlueprintValue(0.0f);
    }

    // 월드 velocity 가져오기
    FVector WorldVelocity = MoveComp->GetVelocity();

    // Owner Actor에서 Forward/Right 벡터 가져오기
    UAnimInstance* AnimInstance = Cast<UAnimInstance>(Context->SourceObject);
    if (!AnimInstance)
    {
        return FBlueprintValue(0.0f);
    }

    USkeletalMeshComponent* MeshComp = AnimInstance->GetOwningComponent();
    if (!MeshComp)
    {
        return FBlueprintValue(0.0f);
    }

    auto* OwnerActor = MeshComp->GetOwner();
    if (!OwnerActor)
    {
        return FBlueprintValue(0.0f);
    }

    FVector Forward = OwnerActor->GetActorForward();
    FVector Right = OwnerActor->GetActorRight();
    FVector Up = OwnerActor->GetActorUp();

    // 월드 -> 로컬 변환 (내적)
    float LocalX = FVector::Dot(WorldVelocity, Right);    // 좌우
    float LocalY = FVector::Dot(WorldVelocity, Forward);  // 앞뒤
    float LocalZ = FVector::Dot(WorldVelocity, Up);       // 위아래

    if (OutputPin->PinName == "X")
    {
        return FBlueprintValue(LocalX);
    }
    else if (OutputPin->PinName == "Y")
    {
        return FBlueprintValue(LocalY);
    }
    else if (OutputPin->PinName == "Z")
    {
        return FBlueprintValue(LocalZ);
    }

    return FBlueprintValue(0.0f);
}

void UK2Node_GetLocalVelocity::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(GetClass());
    Spawner->MenuName = GetNodeTitle();
    Spawner->Category = GetMenuCategory();
    ActionRegistrar.AddAction(Spawner);
}

// ----------------------------------------------------------------
//	[GetSpeed]
// ----------------------------------------------------------------

IMPLEMENT_CLASS(UK2Node_GetSpeed)

UK2Node_GetSpeed::UK2Node_GetSpeed()
{
    TitleColor = ImColor(100, 200, 100);
}

void UK2Node_GetSpeed::AllocateDefaultPins()
{
    // 속력은 스칼라 값이므로 Float 핀 하나만 생성
    CreatePin(EEdGraphPinDirection::EGPD_Output, FEdGraphPinCategory::Float, "Speed");
}

FBlueprintValue UK2Node_GetSpeed::EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context)
{
    auto* MoveComp = GetMovementFromContext(Context);

    // 컴포넌트가 없으면 속력 0 반환
    if (!MoveComp)
    {
        return FBlueprintValue(0.0f);
    }

    if (OutputPin->PinName == "Speed")
    {
        // Velocity 벡터의 길이를 구해서 반환 (FVector::Length() 가정)
        float Speed = MoveComp->GetVelocity().Size();
        //UE_LOG("UK2Node_GetSpeed's Speed: %.2f", Speed);
        return FBlueprintValue(Speed);
    }

    return FBlueprintValue(0.0f);
}

void UK2Node_GetSpeed::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(GetClass());
    Spawner->MenuName = GetNodeTitle();
    Spawner->Category = GetMenuCategory();
    ActionRegistrar.AddAction(Spawner);
}

// ----------------------------------------------------------------
//	[GetIsFinishAnim]
// ----------------------------------------------------------------

IMPLEMENT_CLASS(UK2Node_GetIsFinishAnim)

UK2Node_GetIsFinishAnim::UK2Node_GetIsFinishAnim()
{
    TitleColor = ImColor(100, 200, 100); // Pure Node Green
}

void UK2Node_GetIsFinishAnim::Serialize(const bool bInIsLoading, JSON& InOutHandle)
{
    UK2Node::Serialize(bInIsLoading, InOutHandle);

    if (bInIsLoading)
    {
        FJsonSerializer::ReadString(InOutHandle, "StateName", StateName);
    }
    else
    {
        InOutHandle["StateName"] = StateName;
    }
}

void UK2Node_GetIsFinishAnim::AllocateDefaultPins()
{
    CreatePin(EEdGraphPinDirection::EGPD_Output, FEdGraphPinCategory::Bool, "Is Finished");
}

void UK2Node_GetIsFinishAnim::RenderBody()
{
    ImGui::PushItemWidth(150.0f);
    ImGui::InputText("State Name", &StateName);
    ImGui::PopItemWidth();
}

FBlueprintValue UK2Node_GetIsFinishAnim::EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context)
{
    if (!Context || !Context->SourceObject)
    {
        return FBlueprintValue(false);
    }

    // Context에서 AnimInstance 가져오기
    UAnimInstance* AnimInstance = Cast<UAnimInstance>(Context->SourceObject);
    if (!AnimInstance)
    {
        return FBlueprintValue(false);
    }

    if (OutputPin->PinName == "Is Finished")
    {
        // StateName이 비어있지 않으면 해당 상태일 때만 검사
        UAnimationStateMachine* StateMachine = nullptr;
        if (!StateName.empty())
        {
            StateMachine = AnimInstance->GetStateMachine();
            if (!StateMachine)
            {
                assert(false);
                return FBlueprintValue(false);
            }
        }
        else
        {
            assert(false);
        }

        const TArray<FAnimationState>& States = StateMachine->GetStates();
        FAnimationState TargetState;
        for (const FAnimationState& State : States)
        {
            if (State.Name.ToString() == StateName)
            {
                TargetState = State;
                break;
			}
        }

		FAnimationPlayState CurrentPlayState = AnimInstance->GetCurrentPlayState();
		FAnimationPlayState BlendTargetState = AnimInstance->GetBlendTargetState();

        FAnimationPlayState PlayState;
        if (CurrentPlayState.PoseProvider == TargetState.PoseProvider)
        {
			PlayState = CurrentPlayState;
        }
        else if (BlendTargetState.PoseProvider == TargetState.PoseProvider)
        {
			PlayState = BlendTargetState;
        }
        else
        {
			//assert(false && "Specified StateName not found in current animation states.");
			return FBlueprintValue(false);
        }


        // 현재 재생 중인지 확인
        /*IAnimPoseProvider* PoseProvider = nullptr;
        PoseProvider = AnimInstance->GetCurrentPlayState().PoseProvider;
        if (AnimInstance->GetBlendTargetState().PoseProvider)
        {
			PoseProvider = AnimInstance->GetBlendTargetState().PoseProvider;
        }

		FAnimationPlayState PlayState = AnimInstance->GetCurrentPlayState();
        if (AnimInstance->GetBlendTargetState().PoseProvider)
        {
			PlayState = AnimInstance->GetBlendTargetState();
        }*/

        if (PlayState.bIsPlaying == false || PlayState.loopCount > 1)
        {
            // UE_LOG("Animation is finished. Name: %s, CurPlaytime: %.2f, Playalength: %.2f, loopCount: %.2f", PoseProvider->GetDominantSequence()->GetFilePath().c_str(), PoseProvider->GetCurrentPlayTime(), PoseProvider->GetPlayLength(), AnimInstance->GetCurrentPlayState().loopCount);
			//UE_LOG("Animation is finished. StateName: %s, CurrentTime: %.2f", StateName.c_str(), PlayState.CurrentTime);
            return FBlueprintValue(true);
        }
    }

    return FBlueprintValue(false);
}

void UK2Node_GetIsFinishAnim::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(GetClass());
    Spawner->MenuName = GetNodeTitle();
    Spawner->Category = GetMenuCategory();
    ActionRegistrar.AddAction(Spawner);
}

// ----------------------------------------------------------------
//	[GetRemainAnimLength]
// ----------------------------------------------------------------

IMPLEMENT_CLASS(UK2Node_GetRemainAnimLength)

UK2Node_GetRemainAnimLength::UK2Node_GetRemainAnimLength()
{
    TitleColor = ImColor(100, 200, 100); // Pure Node Green
}

void UK2Node_GetRemainAnimLength::Serialize(const bool bInIsLoading, JSON& InOutHandle)
{
    UK2Node::Serialize(bInIsLoading, InOutHandle);

    if (bInIsLoading)
    {
        FJsonSerializer::ReadString(InOutHandle, "StateName", StateName);
    }
    else
    {
        InOutHandle["StateName"] = StateName;
    }
}

void UK2Node_GetRemainAnimLength::AllocateDefaultPins()
{
    CreatePin(EEdGraphPinDirection::EGPD_Output, FEdGraphPinCategory::Float, "Remain Length");
}

void UK2Node_GetRemainAnimLength::RenderBody()
{
    ImGui::PushItemWidth(150.0f);
    ImGui::InputText("State Name", &StateName);
    ImGui::PopItemWidth();
}

FBlueprintValue UK2Node_GetRemainAnimLength::EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context)
{
    if (!Context || !Context->SourceObject)
    {
        return FBlueprintValue(100.0f);
    }

    // Context에서 AnimInstance 가져오기
    UAnimInstance* AnimInstance = Cast<UAnimInstance>(Context->SourceObject);
    if (!AnimInstance)
    {
        return FBlueprintValue(100.0f);
    }

    if (OutputPin->PinName == "Remain Length")
    {
        // StateName이 비어있지 않으면 해당 상태일 때만 검사
        UAnimationStateMachine* StateMachine = nullptr;
        if (!StateName.empty())
        {
            StateMachine = AnimInstance->GetStateMachine();
            if (!StateMachine)
            {
                assert(false);
                return FBlueprintValue(100.0f);
            }
        }
        else
        {
            assert(false);
        }

        const TArray<FAnimationState>& States = StateMachine->GetStates();
        FAnimationState TargetState;
        for (const FAnimationState& State : States)
        {
            if (State.Name.ToString() == StateName)
            {
                TargetState = State;
                break;
            }
        }

        FAnimationPlayState CurrentPlayState = AnimInstance->GetCurrentPlayState();
        FAnimationPlayState BlendTargetState = AnimInstance->GetBlendTargetState();

        FAnimationPlayState PlayState;
		bool bIsCurrentState = false;
        if (CurrentPlayState.PoseProvider == TargetState.PoseProvider || CurrentPlayState.Sequence == TargetState.Sequence)
        {
            PlayState = CurrentPlayState;
			bIsCurrentState = true;
        }
        else if (BlendTargetState.PoseProvider == TargetState.PoseProvider || BlendTargetState.Sequence == TargetState.Sequence)
        {
            PlayState = BlendTargetState;
			bIsCurrentState = false;
        }
        else
        {
            //assert(false && "Specified StateName not found in current animation states.");
			//UE_LOG("Specified StateName not found in current animation states: %s", StateName.c_str());
            return FBlueprintValue(100.0f);
        }

        // StateName이 비어있지 않으면 해당 상태일 때만 검사
        //if (!StateName.empty())
        //{
        //    UAnimationStateMachine* StateMachine = AnimInstance->GetStateMachine();
        //    if (!StateMachine)
        //    {
        //        return FBlueprintValue(0.0f);
        //    }

        //    // 현재 상태가 지정된 상태와 다르면 0 반환
        //    FName CurrentState = StateMachine->GetCurrentState();
        //    if (CurrentState.ToString() != StateName)
        //    {
        //        return FBlueprintValue(0.0f);
        //    }
        //}

        //FAnimationPlayState PlayState = AnimInstance->GetCurrentPlayState();
        //if (AnimInstance->GetBlendTargetState().PoseProvider)
        //{
        //    PlayState = AnimInstance->GetBlendTargetState();
        //}

        
        // Sequence 직접 사용
        if (PlayState.Sequence)
        {
            float PlayLength = PlayState.Sequence->GetPlayLength();
            float CurrentTime = PlayState.CurrentTime;
            float RemainTime = PlayLength - CurrentTime;
            
            // 음수 방지
            if (RemainTime < 0.0f)
            {
                RemainTime = 0.0f;
            }

			//UE_LOG("Remain Time from Sequence: %.2f, State Name: %s, bIsCurrent: %s", RemainTime, StateName.c_str(), bIsCurrentState ? "true" : "false");
            
            return FBlueprintValue(RemainTime);
        }

        // PoseProvider가 있으면 그것을 사용 (BlendSpace 등)
        if (PlayState.PoseProvider)
        {
            float PlayLength = PlayState.PoseProvider->GetPlayLength();
            float CurrentTime = PlayState.CurrentTime;
            float RemainTime = PlayLength - CurrentTime;
            
            // 음수 방지
            if (RemainTime < 0.0f)
            {
                RemainTime = 0.0f;
            }

            //UE_LOG("Remain Time from Sequence: %.2f, State Name: %s, bIsCurrent: %s", RemainTime, StateName.c_str(), bIsCurrentState ? "true" : "false");
            
            return FBlueprintValue(RemainTime);
        }
    }

    return FBlueprintValue(100.0f);
}

void UK2Node_GetRemainAnimLength::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(GetClass());
    Spawner->MenuName = GetNodeTitle();
    Spawner->Category = GetMenuCategory();
    ActionRegistrar.AddAction(Spawner);
}

// ----------------------------------------------------------------
//	[GetForwardObjHeight] 
// ----------------------------------------------------------------

IMPLEMENT_CLASS(UK2Node_GetForwardObjHeight)

UK2Node_GetForwardObjHeight::UK2Node_GetForwardObjHeight()
{
    TitleColor = ImColor(100, 200, 100); // Pure Node Green
}

void UK2Node_GetForwardObjHeight::AllocateDefaultPins()
{
    CreatePin(EEdGraphPinDirection::EGPD_Output, FEdGraphPinCategory::Float, "Height");
    CreatePin(EEdGraphPinDirection::EGPD_Output, FEdGraphPinCategory::Bool, "Has Object");
}

FBlueprintValue UK2Node_GetForwardObjHeight::EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context)
{
    // 기본값 설정
    float ResultHeight = 1000.0f;
    bool bHasObject = false;

    if (!Context || !Context->SourceObject)
    {
        if (OutputPin->PinName == "Height")
            return FBlueprintValue(ResultHeight);
        else if (OutputPin->PinName == "Has Object")
            return FBlueprintValue(bHasObject);
        return FBlueprintValue{};
    }

    // Context에서 AnimInstance 가져오기
    UAnimInstance* AnimInstance = Cast<UAnimInstance>(Context->SourceObject);
    if (!AnimInstance)
    {
        if (OutputPin->PinName == "Height")
            return FBlueprintValue(ResultHeight);
        else if (OutputPin->PinName == "Has Object")
            return FBlueprintValue(bHasObject);
        return FBlueprintValue{};
    }

    USkeletalMeshComponent* MeshComp = AnimInstance->GetOwningComponent();
    if (!MeshComp)
    {
        if (OutputPin->PinName == "Height")
            return FBlueprintValue(ResultHeight);
        else if (OutputPin->PinName == "Has Object")
            return FBlueprintValue(bHasObject);
        return FBlueprintValue{};
    }

    AActor* OwnerActor = MeshComp->GetOwner();
    if (!OwnerActor)
    {
        if (OutputPin->PinName == "Height")
            return FBlueprintValue(ResultHeight);
        else if (OutputPin->PinName == "Has Object")
            return FBlueprintValue(bHasObject);
        return FBlueprintValue{};
    }

    // Character 캐스팅
    ACharacter* Character = Cast<ACharacter>(OwnerActor);
    if (!Character)
    {
        if (OutputPin->PinName == "Height")
            return FBlueprintValue(ResultHeight);
        else if (OutputPin->PinName == "Has Object")
            return FBlueprintValue(bHasObject);
        return FBlueprintValue{};
    }

    // 캡슐 컴포넌트에서 바닥 Z값 계산
    UCapsuleComponent* CapsuleComp = Character->GetCapsuleComponent();
    if (!CapsuleComp)
    {
        if (OutputPin->PinName == "Height")
            return FBlueprintValue(ResultHeight);
        else if (OutputPin->PinName == "Has Object")
            return FBlueprintValue(bHasObject);
        return FBlueprintValue{};
    }

    // 캡슐 정보 가져오기
    float CapsuleRadius = CapsuleComp->CapsuleRadius;
    float CapsuleHalfHeight = CapsuleComp->CapsuleHalfHeight;
    FVector CapsuleLocation = CapsuleComp->GetWorldLocation();
    float CapsuleBottomZ = CapsuleLocation.Z - CapsuleHalfHeight;

    // 캐릭터 전방 방향
    FVector ForwardDir = OwnerActor->GetActorRight();

    // PhysScene 가져오기
    UWorld* World = OwnerActor->GetWorld();
    if (!World || !World->GetPhysScene())
    {
        if (OutputPin->PinName == "Height")
            return FBlueprintValue(ResultHeight);
        else if (OutputPin->PinName == "Has Object")
            return FBlueprintValue(bHasObject);
        return FBlueprintValue{};
    }

    FPhysScene* PhysScene = World->GetPhysScene();

    // 검사할 박스 영역 설정
    // 캐릭터 전방 0.0 ~ 1.0 거리 (캡슐 반지름 * 2 정도로 설정)
    float SearchDistance = CapsuleRadius * 2.0f; // 약 1.0 거리
    float BoxHalfExtentX = SearchDistance * 0.5f; // 전방 방향 반크기
    float BoxHalfExtentY = CapsuleRadius; // 좌우 폭
    float BoxHalfExtentZ = CapsuleHalfHeight * 1.2f; // 높이 (캡슐 높이의 2배까지 검사)

    // 박스 중심 위치 (캡슐 앞쪽 중간 지점)
	FVector BoxCenter = CapsuleLocation + ForwardDir * (CapsuleRadius + BoxHalfExtentX) + FVector(0.0f, 0.0f, 0.2f);

    // 박스를 캐릭터 방향에 맞춰 회전
    FQuat BoxRotation = OwnerActor->GetActorRotation();

    // PhysX Overlap 쿼리 수행
    TArray<FAABB> OverlappedBounds;
    bool bHasOverlap = PhysScene->OverlapBoxGetBounds(
        BoxCenter,
        FVector(BoxHalfExtentX, BoxHalfExtentY, BoxHalfExtentZ),
        BoxRotation,
        OverlappedBounds,
        OwnerActor // 자기 자신 무시
    );

    if (bHasOverlap && OverlappedBounds.Num() > 0)
    {
        bHasObject = true;

        // 가장 가까운 오브젝트 찾기 (X,Y 기준 캐릭터와 가장 가까운)
        float MinDistanceSq = FLT_MAX;
        float ClosestMaxZ = CapsuleBottomZ;

        for (const FAABB& Bounds : OverlappedBounds)
        {
            // 오브젝트 중심까지의 XY 평면 거리 계산
            FVector ObjCenter = Bounds.GetCenter();
            FVector ToObj = ObjCenter - CapsuleLocation;
            ToObj.Z = 0.0f; // XY 평면에서만 거리 계산
            float DistSq = ToObj.SizeSquared();

            if (DistSq < MinDistanceSq)
            {
                MinDistanceSq = DistSq;
                ClosestMaxZ = Bounds.Max.Z;
            }
        }

        // 높이 계산: 오브젝트 최대 Z - 캡슐 바닥 Z
        ResultHeight = ClosestMaxZ - CapsuleBottomZ;
        
        // 음수 방지 (오브젝트가 캡슐 바닥보다 아래에 있는 경우)
        if (ResultHeight < 0.0f)
        {
            ResultHeight = 0.0f;
        }
    }

    if (OutputPin->PinName == "Height")
    {
		//UE_LOG("UK2Node_GetForwardObjHeight: ResultHeight = %.2f, HasObject = %s", ResultHeight, bHasObject ? "True" : "False");
        return FBlueprintValue(ResultHeight);
    }
    else if (OutputPin->PinName == "Has Object")
    {
        return FBlueprintValue(bHasObject);
    }

    return FBlueprintValue{};
}

void UK2Node_GetForwardObjHeight::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(GetClass());
    Spawner->MenuName = GetNodeTitle();
    Spawner->Category = GetMenuCategory();
    ActionRegistrar.AddAction(Spawner);
}