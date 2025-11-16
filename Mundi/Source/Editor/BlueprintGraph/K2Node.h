#pragma once

#include "EdGraphNode.h"

class FBlueprintActionDatabaseRegistrar;
    
UCLASS(DisplayName="UK2Node", Description="블루프린트 노드")
class UK2Node : public UEdGraphNode 
{
    DECLARE_CLASS(UK2Node, UEdGraphNode)
public:

    /** @brief 노드 타입이 순수 노드(실행 핀이 없음)인지 여부 */
    virtual bool IsNodePure() const { return false; }

    /** @brief 컨텍스트 메뉴에 이 노드를 생성하는 액션을 등록한다. */
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const {}

    /** @brief 이 노드 타입이 속할 메뉴 카테고리를 반환한다. */
    virtual FString GetMenuCategory() const { return "Default"; }
};
