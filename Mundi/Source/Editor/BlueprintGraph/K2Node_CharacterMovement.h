/**
* ===========================================================================
 * @file      K2Node_CharacterMovement.h
 * @author    geb0598
 * @date      2025/11/19
 * @brief     캐릭터 움직임과 관련된 블루프린트 노드를 정의한다.
 *
 * ===========================================================================
 *
 * @note 현재 Context는 UAnimInstance타입이라고 가정한다. 따라서, UAnimInstance에서
 *       출발하여 원하는 컴포넌트를 찾을 때까지 계층을 돌며 탐색한다.
 *
 * ===========================================================================
 */

#pragma once

#include "K2Node.h"

// ----------------------------------------------------------------
//	[GetIsFalling] 캐릭터 낙하 상태 확인 노드
// ----------------------------------------------------------------
UCLASS(DisplayName = "Is Falling", Description = "캐릭터가 현재 공중에 떠있는지(낙하 중인지) 확인합니다.")
class UK2Node_GetIsFalling : public UK2Node
{
    DECLARE_CLASS(UK2Node_GetIsFalling, UK2Node);

public:
    UK2Node_GetIsFalling();

    // --- UEdGraphNode 인터페이스 ---
public:
    virtual FString GetNodeTitle() const override { return "Is Falling"; }
    virtual bool IsNodePure() const override { return true; }
    virtual void AllocateDefaultPins() override;
    virtual FBlueprintValue EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context) override;

    // --- UK2Node 인터페이스 ---
public:
    virtual FString GetMenuCategory() const override { return "캐릭터 무브먼트"; };
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};

// ----------------------------------------------------------------
//	[GetIsSliding] 캐릭터 슬라이딩 상태 확인 노드
// ----------------------------------------------------------------
UCLASS(DisplayName = "Is Sliding", Description = "캐릭터가 현재 공중에 떠있는지(낙하 중인지) 확인합니다.")
class UK2Node_GetIsSliding : public UK2Node
{
    DECLARE_CLASS(UK2Node_GetIsSliding, UK2Node);

public:
    UK2Node_GetIsSliding();

    // --- UEdGraphNode 인터페이스 ---
public:
    virtual FString GetNodeTitle() const override { return "Is Sliding"; }
    virtual bool IsNodePure() const override { return true; }
    virtual void AllocateDefaultPins() override;
    virtual FBlueprintValue EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context) override;

    // --- UK2Node 인터페이스 ---
public:
    virtual FString GetMenuCategory() const override { return "캐릭터 무브먼트"; };
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};

// ----------------------------------------------------------------
//	[GetIsJumping] 캐릭터 슬라이딩 상태 확인 노드
// ----------------------------------------------------------------
UCLASS(DisplayName = "Is Jumping", Description = "캐릭터가 현재 공중에 떠있는지(낙하 중인지) 확인합니다.")
class UK2Node_GetIsJumping : public UK2Node
{
    DECLARE_CLASS(UK2Node_GetIsJumping, UK2Node);

public:
    UK2Node_GetIsJumping();

    // --- UEdGraphNode 인터페이스 ---
public:
    virtual FString GetNodeTitle() const override { return "Is Jumping"; }
    virtual bool IsNodePure() const override { return true; }
    virtual void AllocateDefaultPins() override;
    virtual FBlueprintValue EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context) override;

    // --- UK2Node 인터페이스 ---
public:
    virtual FString GetMenuCategory() const override { return "캐릭터 무브먼트"; };
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};

// ----------------------------------------------------------------
//	[GetNeedRolling] 캐릭터 슬라이딩 상태 확인 노드
// ----------------------------------------------------------------
UCLASS(DisplayName = "Need Rolling", Description = "캐릭터가 현재 공중에 떠있는지(낙하 중인지) 확인합니다.")
class UK2Node_GetNeedRolling : public UK2Node
{
    DECLARE_CLASS(UK2Node_GetNeedRolling, UK2Node);

public:
    UK2Node_GetNeedRolling();

    // --- UEdGraphNode 인터페이스 ---
public:
    virtual FString GetNodeTitle() const override { return "Need Rolling"; }
    virtual bool IsNodePure() const override { return true; }
    virtual void AllocateDefaultPins() override;
    virtual FBlueprintValue EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context) override;

    // --- UK2Node 인터페이스 ---
public:
    virtual FString GetMenuCategory() const override { return "캐릭터 무브먼트"; };
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};

// ----------------------------------------------------------------
//	[GetVelocity] 이동 속도 벡터 반환 노드
// ----------------------------------------------------------------
UCLASS(DisplayName = "Get Velocity", Description = "캐릭터의 현재 이동 속도 벡터(X, Y, Z)를 반환합니다.")
class UK2Node_GetVelocity : public UK2Node
{
    DECLARE_CLASS(UK2Node_GetVelocity, UK2Node);

public:
    UK2Node_GetVelocity();

    // --- UEdGraphNode 인터페이스 ---
public:
    virtual FString GetNodeTitle() const override { return "Get Velocity"; }
    virtual bool IsNodePure() const override { return true; }
    virtual void AllocateDefaultPins() override;
    virtual FBlueprintValue EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context) override;

    // --- UK2Node 인터페이스 ---
public:
    virtual FString GetMenuCategory() const override { return "캐릭터 무브먼트"; };
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};

// ----------------------------------------------------------------
//	[GetLocalVelocity] 로컬 좌표계 이동 속도 벡터 반환 노드
// ----------------------------------------------------------------
UCLASS(DisplayName = "Get Local Velocity", Description = "캐릭터가 바라보는 방향 기준 로컬 속도 벡터를 반환합니다. Blend Space 2D에 사용하기 적합합니다.")
class UK2Node_GetLocalVelocity : public UK2Node
{
    DECLARE_CLASS(UK2Node_GetLocalVelocity, UK2Node);

public:
    UK2Node_GetLocalVelocity();

    // --- UEdGraphNode 인터페이스 ---
public:
    virtual FString GetNodeTitle() const override { return "Get Local Velocity"; }
    virtual bool IsNodePure() const override { return true; }
    virtual void AllocateDefaultPins() override;
    virtual FBlueprintValue EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context) override;

    // --- UK2Node 인터페이스 ---
public:
    virtual FString GetMenuCategory() const override { return "캐릭터 무브먼트"; };
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};

// ----------------------------------------------------------------
//	[GetSpeed] 이동 속력(Scalar) 반환 노드
// ----------------------------------------------------------------
UCLASS(DisplayName = "Get Speed", Description = "캐릭터의 현재 이동 속력(Speed)을 반환한다.")
class UK2Node_GetSpeed : public UK2Node
{
    DECLARE_CLASS(UK2Node_GetSpeed, UK2Node);

public:
    UK2Node_GetSpeed();

    // --- UEdGraphNode 인터페이스 ---
public:
    virtual FString GetNodeTitle() const override { return "Get Speed"; }
    virtual bool IsNodePure() const override { return true; }
    virtual void AllocateDefaultPins() override;
    virtual FBlueprintValue EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context) override;

    // --- UK2Node 인터페이스 ---
public:
    virtual FString GetMenuCategory() const override { return "캐릭터 무브먼트"; };
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};

// ----------------------------------------------------------------
//	[GetIsFinishAnim] 애니메이션 종료 여부 확인 노드
// ----------------------------------------------------------------
UCLASS(DisplayName = "Is Animation Finished", Description = "현재 재생 중인 애니메이션이 끝 시간에 도달했는지 확인합니다.")
class UK2Node_GetIsFinishAnim : public UK2Node
{
    DECLARE_CLASS(UK2Node_GetIsFinishAnim, UK2Node);

public:
    UK2Node_GetIsFinishAnim();

    // --- UEdGraphNode 인터페이스 ---
public:
    virtual FString GetNodeTitle() const override { return "Is Animation Finished"; }
    virtual bool IsNodePure() const override { return true; }
    virtual void AllocateDefaultPins() override;
    virtual FBlueprintValue EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context) override;

    // --- UK2Node 인터페이스 ---
public:
    virtual FString GetMenuCategory() const override { return "애니메이션"; };
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};

// ----------------------------------------------------------------
//	[GetRemainAnimLength] 애니메이션 남은 시간 반환 노드
// ----------------------------------------------------------------
UCLASS(DisplayName = "Get Remain Animation Length", Description = "현재 재생 중인 애니메이션의 남은 시간을 반환합니다.")
class UK2Node_GetRemainAnimLength : public UK2Node
{
    DECLARE_CLASS(UK2Node_GetRemainAnimLength, UK2Node);

public:
    UK2Node_GetRemainAnimLength();

    // --- UEdGraphNode 인터페이스 ---
public:
    virtual FString GetNodeTitle() const override { return "Get Remain Animation Length"; }
    virtual bool IsNodePure() const override { return true; }
    virtual void AllocateDefaultPins() override;
    virtual FBlueprintValue EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context) override;

    // --- UK2Node 인터페이스 ---
public:
    virtual FString GetMenuCategory() const override { return "애니메이션"; };
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};

// ----------------------------------------------------------------
//	[GetForwardObjHeight] 캐릭터 앞 오브젝트 높이 확인 노드
// ----------------------------------------------------------------
UCLASS(DisplayName = "Get Forward Object Height", Description = "캐릭터 앞 방향의 가장 가까운 오브젝트의 높이를 반환합니다. 벽 등반 가능 여부 판단에 사용됩니다.")
class UK2Node_GetForwardObjHeight : public UK2Node
{
    DECLARE_CLASS(UK2Node_GetForwardObjHeight, UK2Node);

public:
    UK2Node_GetForwardObjHeight();

    // --- UEdGraphNode 인터페이스 ---
public:
    virtual FString GetNodeTitle() const override { return "Get Forward Object Height"; }
    virtual bool IsNodePure() const override { return true; }
    virtual void AllocateDefaultPins() override;
    virtual FBlueprintValue EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context) override;

    // --- UK2Node 인터페이스 ---
public:
    virtual FString GetMenuCategory() const override { return "캐릭터 무브먼트"; };
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};

