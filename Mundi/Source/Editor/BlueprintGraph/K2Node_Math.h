/**
* ===========================================================================
 * @file      K2Node_Math.h
 * @author    geb0598
 * @date      2025/11/17
 * @brief     블루프린트에서 사용되는 수학 연산 노드(Add, Subtract, Multiply 등)를 정의한다.
 *
 * ===========================================================================
 *
 * @note 코딩 규칙
 *   - 입력 핀 이름은 순서대로 'A', 'B', 'C' ... 로 지정한다.
 *   - 출력 핀 이름은 'Result'로 지정한다.
 *   - 노드 클래스 이름은 "UK2Node_[연산]_[타입][타입]" 형식으로 작성한다.
 *     예: UK2Node_Add_FloatFloat
 *   - 노드 UI에서 표시되는 제목은 연산 기호와 타입을 포함한다.
 *     예: "+ (Float)", "- (Float)", "* (Float)"
 *
 * ===========================================================================
 */

#pragma once

#include "K2Node.h"

// ----------------------------------------------------------------
//	[Float] 노드
// ----------------------------------------------------------------

UCLASS(DisplayName="UK2Node_Add_FloatFloat", Description="블루프린트 float 덧셈 노드")
class UK2Node_Add_FloatFloat : public UK2Node
{
    DECLARE_CLASS(UK2Node_Add_FloatFloat, UK2Node);

public:
    UK2Node_Add_FloatFloat();

    // --- UEdGraphNode 인터페이스 ---
public:
    virtual FString GetNodeTitle() const override { return "+ (Float)"; }
    virtual bool IsNodePure() const override { return true; }
    virtual void AllocateDefaultPins() override;
    virtual void RenderBody() override;
    virtual FBlueprintValue EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context) override;
    
    // --- UK2Node 인터페이스 ---
public:
    virtual FString GetMenuCategory() const override { return "수학"; }
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};

UCLASS(DisplayName="UK2Node_Subtract_FloatFloat", Description="블루프린트 float 뺄셈 노드")
class UK2Node_Subtract_FloatFloat : public UK2Node
{
    DECLARE_CLASS(UK2Node_Subtract_FloatFloat, UK2Node);

public:
    UK2Node_Subtract_FloatFloat();

    // --- UEdGraphNode 인터페이스 ---
public:
    virtual FString GetNodeTitle() const override { return "- (Float)"; }
    virtual bool IsNodePure() const override { return true; }
    virtual void AllocateDefaultPins() override;
    virtual void RenderBody() override;
    virtual FBlueprintValue EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context) override;

    // --- UK2Node 인터페이스 ---
public:
    virtual FString GetMenuCategory() const override { return "수학"; }
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};

UCLASS(DisplayName="UK2Node_Multiply_FloatFloat", Description="블루프린트 float 곱셈 노드")
class UK2Node_Multiply_FloatFloat : public UK2Node
{
    DECLARE_CLASS(UK2Node_Multiply_FloatFloat, UK2Node);

public:
    UK2Node_Multiply_FloatFloat();

    // --- UEdGraphNode 인터페이스 ---
public:
    virtual FString GetNodeTitle() const override { return "* (Float)"; }
    virtual bool IsNodePure() const override { return true; }
    virtual void AllocateDefaultPins() override;
    virtual void RenderBody() override;
    virtual FBlueprintValue EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context) override;
    
    // --- UK2Node 인터페이스 ---
public:
    virtual FString GetMenuCategory() const override { return "수학"; }
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};

UCLASS(DisplayName = "UK2Node_Divide_FloatFloat", Description = "블루프린트 float 나눗셈 노드")
class UK2Node_Divide_FloatFloat : public UK2Node
{
    DECLARE_CLASS(UK2Node_Divide_FloatFloat, UK2Node);

public:
    UK2Node_Divide_FloatFloat();

    // --- UEdGraphNode 인터페이스 ---
public:
    virtual FString GetNodeTitle() const override { return "/ (Float)"; }
    virtual bool IsNodePure() const override { return true; }
    virtual void AllocateDefaultPins() override;
    virtual void RenderBody() override;
    virtual FBlueprintValue EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context) override;

    // --- UK2Node 인터페이스 ---
public:
    virtual FString GetMenuCategory() const override { return "수학"; }
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};

UCLASS(DisplayName = "UK2Node_Greater_FloatFloat", Description = "블루프린트 float '>' 노드")
class UK2Node_Greater_FloatFloat : public UK2Node
{
    DECLARE_CLASS(UK2Node_Greater_FloatFloat, UK2Node);

public:
    UK2Node_Greater_FloatFloat();

    // --- UEdGraphNode 인터페이스 ---
public:
    virtual FString GetNodeTitle() const override { return "> (Float)"; }
    virtual bool IsNodePure() const override { return true; }
    virtual void AllocateDefaultPins() override;
    virtual void RenderBody() override;
    virtual FBlueprintValue EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context) override;

    // --- UK2Node 인터페이스 ---
public:
    virtual FString GetMenuCategory() const override { return "수학"; }
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};

UCLASS(DisplayName = "UK2Node_Equal_FloatFloat", Description = "블루프린트 float '==' 노드")
class UK2Node_Equal_FloatFloat : public UK2Node
{
    DECLARE_CLASS(UK2Node_Equal_FloatFloat, UK2Node);

public:
    UK2Node_Equal_FloatFloat();

    // --- UEdGraphNode 인터페이스 ---
public:
    virtual FString GetNodeTitle() const override { return "== (Float)"; }
    virtual bool IsNodePure() const override { return true; }
    virtual void AllocateDefaultPins() override;
    virtual void RenderBody() override;
    virtual FBlueprintValue EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context) override;

    // --- UK2Node 인터페이스 ---
public:
    virtual FString GetMenuCategory() const override { return "수학"; }
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};

// ----------------------------------------------------------------
//	[Int] 노드
// ----------------------------------------------------------------

UCLASS(DisplayName = "UK2Node_Add_IntInt", Description = "블루프린트 int 덧셈 노드")
class UK2Node_Add_IntInt : public UK2Node
{
    DECLARE_CLASS(UK2Node_Add_IntInt, UK2Node);

public:
    UK2Node_Add_IntInt();

    // --- UEdGraphNode 인터페이스 ---
public:
    virtual FString GetNodeTitle() const override { return "+ (Int)"; }
    virtual bool IsNodePure() const override { return true; }
    virtual void AllocateDefaultPins() override;
    virtual void RenderBody() override;
    virtual FBlueprintValue EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context) override;

    // --- UK2Node 인터페이스 ---
public:
    virtual FString GetMenuCategory() const override { return "수학"; }
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};

UCLASS(DisplayName = "UK2Node_Subtract_IntInt", Description = "블루프린트 int 뺄셈 노드")
class UK2Node_Subtract_IntInt : public UK2Node
{
    DECLARE_CLASS(UK2Node_Subtract_IntInt, UK2Node);

public:
    UK2Node_Subtract_IntInt();

    // --- UEdGraphNode 인터페이스 ---
public:
    virtual FString GetNodeTitle() const override { return "- (Int)"; }
    virtual bool IsNodePure() const override { return true; }
    virtual void AllocateDefaultPins() override;
    virtual void RenderBody() override;
    virtual FBlueprintValue EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context) override;

    // --- UK2Node 인터페이스 ---
public:
    virtual FString GetMenuCategory() const override { return "수학"; }
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};

UCLASS(DisplayName = "UK2Node_Multiply_IntInt", Description = "블루프린트 int 곱셈 노드")
class UK2Node_Multiply_IntInt : public UK2Node
{
    DECLARE_CLASS(UK2Node_Multiply_IntInt, UK2Node);

public:
    UK2Node_Multiply_IntInt();

    // --- UEdGraphNode 인터페이스 ---
public:
    virtual FString GetNodeTitle() const override { return "* (Int)"; }
    virtual bool IsNodePure() const override { return true; }
    virtual void AllocateDefaultPins() override;
    virtual void RenderBody() override;
    virtual FBlueprintValue EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context) override;

    // --- UK2Node 인터페이스 ---
public:
    virtual FString GetMenuCategory() const override { return "수학"; }
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};

UCLASS(DisplayName = "UK2Node_Divide_IntInt", Description = "블루프린트 int 나눗셈 노드")
class UK2Node_Divide_IntInt : public UK2Node
{
    DECLARE_CLASS(UK2Node_Divide_IntInt, UK2Node);

public:
    UK2Node_Divide_IntInt();

    // --- UEdGraphNode 인터페이스 ---
public:
    virtual FString GetNodeTitle() const override { return "/ (Int)"; }
    virtual bool IsNodePure() const override { return true; }
    virtual void AllocateDefaultPins() override;
    virtual void RenderBody() override;
    virtual FBlueprintValue EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context) override;

    // --- UK2Node 인터페이스 ---
public:
    virtual FString GetMenuCategory() const override { return "수학"; }
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};

UCLASS(DisplayName = "UK2Node_Greater_IntInt", Description = "블루프린트 int '>' 노드")
class UK2Node_Greater_IntInt : public UK2Node
{
    DECLARE_CLASS(UK2Node_Greater_IntInt, UK2Node);

public:
    UK2Node_Greater_IntInt();

    // --- UEdGraphNode 인터페이스 ---
public:
    virtual FString GetNodeTitle() const override { return "> (Int)"; }
    virtual bool IsNodePure() const override { return true; }
    virtual void AllocateDefaultPins() override;
    virtual void RenderBody() override;
    virtual FBlueprintValue EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context) override;

    // --- UK2Node 인터페이스 ---
public:
    virtual FString GetMenuCategory() const override { return "수학"; }
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};

UCLASS(DisplayName = "UK2Node_Equal_IntInt", Description = "블루프린트 int '==' 노드")
class UK2Node_Equal_IntInt : public UK2Node
{
    DECLARE_CLASS(UK2Node_Equal_IntInt, UK2Node);

public:
    UK2Node_Equal_IntInt();

    // --- UEdGraphNode 인터페이스 ---
public:
    virtual FString GetNodeTitle() const override { return "== (Int)"; }
    virtual bool IsNodePure() const override { return true; }
    virtual void AllocateDefaultPins() override;
    virtual void RenderBody() override;
    virtual FBlueprintValue EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context) override;

    // --- UK2Node 인터페이스 ---
public:
    virtual FString GetMenuCategory() const override { return "수학"; }
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};

// ----------------------------------------------------------------
//	[Bool] 노드 (논리 연산)
// ----------------------------------------------------------------

UCLASS(DisplayName = "UK2Node_And_BoolBool", Description = "블루프린트 AND 논리 연산 노드")
class UK2Node_And_BoolBool : public UK2Node
{
    DECLARE_CLASS(UK2Node_And_BoolBool, UK2Node);

public:
    UK2Node_And_BoolBool();

    // --- UEdGraphNode 인터페이스 ---
public:
    virtual FString GetNodeTitle() const override { return "AND (Bool)"; }
    virtual bool IsNodePure() const override { return true; }
    virtual void AllocateDefaultPins() override;
    virtual void RenderBody() override;
    virtual FBlueprintValue EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context) override;

    // --- UK2Node 인터페이스 ---
public:
    virtual FString GetMenuCategory() const override { return "수학"; }
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};

UCLASS(DisplayName = "UK2Node_Or_BoolBool", Description = "블루프린트 OR 논리 연산 노드")
class UK2Node_Or_BoolBool : public UK2Node
{
    DECLARE_CLASS(UK2Node_Or_BoolBool, UK2Node);

public:
    UK2Node_Or_BoolBool();

    // --- UEdGraphNode 인터페이스 ---
public:
    virtual FString GetNodeTitle() const override { return "OR (Bool)"; }
    virtual bool IsNodePure() const override { return true; }
    virtual void AllocateDefaultPins() override;
    virtual void RenderBody() override;
    virtual FBlueprintValue EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context) override;

    // --- UK2Node 인터페이스 ---
public:
    virtual FString GetMenuCategory() const override { return "수학"; }
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};

UCLASS(DisplayName = "UK2Node_Xor_BoolBool", Description = "블루프린트 XOR 논리 연산 노드")
class UK2Node_Xor_BoolBool : public UK2Node
{
    DECLARE_CLASS(UK2Node_Xor_BoolBool, UK2Node);

public:
    UK2Node_Xor_BoolBool();

    // --- UEdGraphNode 인터페이스 ---
public:
    virtual FString GetNodeTitle() const override { return "XOR (Bool)"; }
    virtual bool IsNodePure() const override { return true; }
    virtual void AllocateDefaultPins() override;
    virtual void RenderBody() override;
    virtual FBlueprintValue EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context) override;

    // --- UK2Node 인터페이스 ---
public:
    virtual FString GetMenuCategory() const override { return "수학"; }
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};

UCLASS(DisplayName = "UK2Node_Not_Bool", Description = "블루프린트 NOT 논리 연산 노드")
class UK2Node_Not_Bool : public UK2Node
{
    DECLARE_CLASS(UK2Node_Not_Bool, UK2Node);

public:
    UK2Node_Not_Bool();

    // --- UEdGraphNode 인터페이스 ---
public:
    virtual FString GetNodeTitle() const override { return "NOT (Bool)"; }
    virtual bool IsNodePure() const override { return true; }
    virtual void AllocateDefaultPins() override;
    virtual void RenderBody() override;
    virtual FBlueprintValue EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context) override;

    // --- UK2Node 인터페이스 ---
public:
    virtual FString GetMenuCategory() const override { return "수학"; }
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};