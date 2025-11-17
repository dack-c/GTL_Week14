/**
* ===========================================================================
 * @file      K2Node_Misc.h
 * @author    geb0598
 * @date      2025/11/17
 * @brief     블루프린트에서 사용되는 기타 유틸리티 노드(입력 처리 등)를 정의한다.
 *
 * ===========================================================================
 *
 * @note 코딩 규칙
 *   - `UK2Node_IsPressed`: 키 입력을 확인하는 노드. `KeyName` 프로퍼티에 확인할 키 이름을 문자열로 지정한다. bool 값을 반환하는 'Result' 출력 핀을 가진다.
 *   - `UK2Node_GetMousePosition`: 마우스 커서의 현재 위치를 얻는 노드. 'X'와 'Y' 출력 핀을 통해 float 값을 반환한다.
 *   - 모든 입력 관련 노드는 "입력" 메뉴 카테고리에 속한다.
 *
 * ===========================================================================
 */

#pragma once

#include "K2Node.h"

// ----------------------------------------------------------------
//	[Input] 키보드 입력 확인 노드
// ----------------------------------------------------------------

/**
 * @todo 현재는 문자열로 키를 처리하지만, 실수를 줄일 수 있는 더 나은 방법을 찾아야 한다.
 */
UCLASS(DisplayName="UK2Node_IsPressed", Description="지정된 키가 현재 눌려있는지 확인합니다 (bool 반환).")
class UK2Node_IsPressed : public UK2Node
{
    DECLARE_CLASS(UK2Node_IsPressed, UK2Node);

public:
    /** 확인할 키의 이름 (예: "Space", "W") */
    FString KeyName = "Space";
    
    // --- UEdGraphNode 인터페이스 ---
public:
    virtual FString GetNodeTitle() const override { return "Is Key Pressed"; }
    virtual bool IsNodePure() const override { return true; } // 순수 노드로 처리
    virtual void AllocateDefaultPins() override;
    virtual void RenderBody() override;
    virtual FBlueprintValue EvaluatePin(const UEdGraphPin* OutputPin) override;
    
    // --- UK2Node 인터페이스 ---
public:
    virtual FString GetMenuCategory() const override { return "입력"; };
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};

// ----------------------------------------------------------------
//	[Input] 마우스 위치 확인 노드
// ----------------------------------------------------------------

UCLASS(DisplayName = "UK2Node_GetMousePosition", Description = "현재 마우스 커서의 위치(X, Y)를 반환합니다.")
class UK2Node_GetMousePosition : public UK2Node
{
    DECLARE_CLASS(UK2Node_GetMousePosition, UK2Node);

    // --- UEdGraphNode 인터페이스 ---
public:
    virtual FString GetNodeTitle() const override { return "Get Mouse Position"; }
    virtual bool IsNodePure() const override { return true; } // 순수 노드로 처리
    virtual void AllocateDefaultPins() override;
    virtual void RenderBody() override; 
    virtual FBlueprintValue EvaluatePin(const UEdGraphPin* OutputPin) override;

    // --- UK2Node 인터페이스 ---
public:
    virtual FString GetMenuCategory() const override { return "입력"; };
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};

// ----------------------------------------------------------------
//	[Debug] 값 확인(Watch) 노드
// ----------------------------------------------------------------

UCLASS(DisplayName="UK2Node_Watch_Int", Description="입력된 Int 값을 노드 위에 실시간으로 표시합니다.")
class UK2Node_Watch_Int : public UK2Node
{
    DECLARE_CLASS(UK2Node_Watch_Int, UK2Node);

    // --- UEdGraphNode 인터페이스 ---
public:
    virtual FString GetNodeTitle() const override { return "Watch (Int)"; }
    virtual bool IsNodePure() const override { return true; }
    virtual void AllocateDefaultPins() override;
    virtual void RenderBody() override;

    // --- UK2Node 인터페이스 ---
public:
    virtual FString GetMenuCategory() const override { return "디버그"; };
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};

UCLASS(DisplayName="UK2Node_Watch_Float", Description="입력된 Float 값을 노드 위에 실시간으로 표시합니다.")
class UK2Node_Watch_Float : public UK2Node
{
    DECLARE_CLASS(UK2Node_Watch_Float, UK2Node);

    // --- UEdGraphNode 인터페이스 ---
public:
    virtual FString GetNodeTitle() const override { return "Watch (Float)"; }
    virtual bool IsNodePure() const override { return true; }
    virtual void AllocateDefaultPins() override;
    virtual void RenderBody() override;

    // --- UK2Node 인터페이스 ---
public:
    virtual FString GetMenuCategory() const override { return "디버그"; };
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};

UCLASS(DisplayName="UK2Node_Watch_Bool", Description="입력된 Bool 값을 노드 위에 실시간으로 표시합니다.")
class UK2Node_Watch_Bool : public UK2Node
{
    DECLARE_CLASS(UK2Node_Watch_Bool, UK2Node);

    // --- UEdGraphNode 인터페이스 ---
public:
    virtual FString GetNodeTitle() const override { return "Watch (Bool)"; }
    virtual bool IsNodePure() const override { return true; }
    virtual void AllocateDefaultPins() override;
    virtual void RenderBody() override;

    // --- UK2Node 인터페이스 ---
public:
    virtual FString GetMenuCategory() const override { return "디버그"; };
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};