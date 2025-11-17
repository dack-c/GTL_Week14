/**
* ===========================================================================
 * @file      K2Node.h
 * @author    geb0598
 * @date      2025/11/17
 * @brief     블루프린트 에디터에서 사용되는 모든 K2Node의 기반 클래스를 정의한다.
 *
 * ===========================================================================
 *
 * @note 사용 주의사항
 *   - 모든 노드는 UK2Node를 상속받아 구현한다.
 *   - 정의된 노드는 RTTI 시스템에 의해 자동으로 컨텍스트 메뉴에 등록된다.
 *   - 노드 이름과 메뉴에 표시되는 이름은 GetNodeTitle()과 GetMenuCategory()를 통해 정의한다.
 *   - 순수 함수로 구현할 경우 IsNodePure()를 true로 설정한다.
 *   - AllocateDefaultPins()에서 입력 및 출력 핀을 정의한다.
 *   - RenderBody()에서 노드 UI를 커스터마이징한다.
 *
 * ===========================================================================
 */

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
