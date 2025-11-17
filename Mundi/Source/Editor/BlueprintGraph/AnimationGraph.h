#pragma once

#include "EdGraph.h"

class UK2Node_AnimStateEntry;

/**
 * @brief 애니메이션 상태 머신 로직을 담는 특수한 그래프
 * @note 항상 하나의 Entry 노드를 유지하도록 보장한다.
 *       하나의 실행흐름 출력핀이 여러개의 입력핀과 연결될 수 있다.
 *       언리얼 엔진에서는 상태 전이에 우선순위를 두어 처리하지만 퓨처 엔진에서는 구현의 용이함을 위해 해당 경우를 무시한다.
 * @todo 사용자가 Entry 노드를 삭제할 수 없도록 처리하거나, 삭제될 시 새로운 엔트리 노드를 생성한다.
 */
UCLASS(DisplayName="UAnimationGraph", Description="애니메이션 그래프")
class UAnimationGraph : public UEdGraph
{
    DECLARE_CLASS(UAnimationGraph, UEdGraph)
public:
    UAnimationGraph();

    virtual ~UAnimationGraph() = default;

    /** @brief 애니메이션 엔트리 노드를 가져온다. */
    UK2Node_AnimStateEntry* GetEntryNode();

    /** @brief 엔트리 노드가 그래프에 존재하는지 확인하고, 없을 경우 새로 생성한다. */
    void ValidateGraph();
};