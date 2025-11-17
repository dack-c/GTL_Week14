/**
* ===========================================================================
 * @file      K2Node_Literal.h
 * @author    geb0598
 * @date      2025/11/17
 * @brief     블루프린트에서 사용되는 리터럴(int, float, bool 등) 노드를 정의한다.
 *
 * ===========================================================================
 *
 * @note 코딩 규칙
 *   - 리터럴 노드는 `UK2Node_Literal_[타입]` 형식의 클래스 이름을 가진다. (예: `UK2Node_Literal_Int`)
 *   - 노드의 UI에 표시되는 제목은 타입의 이름과 동일하다. (예: "int", "float")
 *   - 모든 리터럴 노드는 순수 함수(`IsNodePure` = true)이다.
 *   - 리터럴 값을 담는 프로퍼티의 이름은 `Value`로 지정한다.
 *   - 출력 핀의 이름은 `Value`로 지정한다.
 *   - 모든 리터럴 노드는 "리터럴" 메뉴 카테고리에 속한다.
 *
 * ===========================================================================
 */

#pragma once

#include "K2Node.h"

// ----------------------------------------------------------------
//	[Int] 리터럴 노드
// ----------------------------------------------------------------

UCLASS(DisplayName="UK2Node_Literal_Int", Description="블루프린트 int 리터럴 노드")
class UK2Node_Literal_Int : public UK2Node
{
    DECLARE_CLASS(UK2Node_Literal_Int, UK2Node);

public:
    /** @todo UPROPERTY 시스템 통합 */
    int32 Value;
    
    // --- UEdGraphNode 인터페이스 ---
public:
    virtual FString GetNodeTitle() const override { return "int"; }
    virtual bool IsNodePure() const override { return true; }
    virtual void AllocateDefaultPins() override;
    virtual void RenderBody() override;
    virtual FBlueprintValue EvaluatePin(const UEdGraphPin* OutputPin) override;
    
    // --- UK2Node 인터페이스 ---
public:
    virtual FString GetMenuCategory() const override { return "리터럴"; };
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};

// ----------------------------------------------------------------
//	[Float] 리터럴 노드
// ----------------------------------------------------------------

UCLASS(DisplayName="UK2Node_Literal_Float", Description="블루프린트 float 리터럴 노드")
class UK2Node_Literal_Float : public UK2Node
{
    DECLARE_CLASS(UK2Node_Literal_Float, UK2Node);

public:
    /** @todo UPROPERTY 시스템 통합 */
    float Value;
    
    // --- UEdGraphNode 인터페이스 ---
public:
    virtual FString GetNodeTitle() const override { return "float"; }
    virtual bool IsNodePure() const override { return true; }
    virtual void AllocateDefaultPins() override;
    virtual void RenderBody() override;
    virtual FBlueprintValue EvaluatePin(const UEdGraphPin* OutputPin) override;
    
    // --- UK2Node 인터페이스 ---
public:
    virtual FString GetMenuCategory() const override { return "리터럴"; };
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};

// ----------------------------------------------------------------
//	[Bool] 리터럴 노드
// ----------------------------------------------------------------

UCLASS(DisplayName="UK2Node_Literal_Bool", Description="블루프린트 bool 리터럴 노드")
class UK2Node_Literal_Bool : public UK2Node
{
    DECLARE_CLASS(UK2Node_Literal_Bool, UK2Node);

public:
    /** @todo UPROPERTY 시스템 통합 */
    bool Value;
    
    // --- UEdGraphNode 인터페이스 ---
public:
    virtual FString GetNodeTitle() const override { return "bool"; }
    virtual bool IsNodePure() const override { return true; }
    virtual void AllocateDefaultPins() override;
    virtual void RenderBody() override;
    virtual FBlueprintValue EvaluatePin(const UEdGraphPin* OutputPin) override;
    
    // --- UK2Node 인터페이스 ---
public:
    virtual FString GetMenuCategory() const override { return "리터럴"; };
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};