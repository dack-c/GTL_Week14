#pragma once
#include "EdGraphPin.h"

UCLASS(DisplayName="UEdGraph", Description="블루프린트 그래프")
class UEdGraph : public UObject
{
    DECLARE_CLASS(UEdGraph, UObject)
    
public:
    /** @brief 그래프에 포함된 노드 목록 */
    TArray<UEdGraphNode*> Nodes;

public:
    UEdGraph() = default;

    virtual ~UEdGraph();

    /** @brief 노드를 그래프에 추가 */
    UEdGraphNode* AddNode(UEdGraphNode* Node);

    /**
     * @brief 노드를 그래프에서 제거
     * @note 그래프에서 제거된 노드는 delete를 통해 해제된다.
     */
    void RemoveNode(UEdGraphNode* Node);

    /**
     * @brief 노드를 그래프에서 제거
     * @note 그래프에서 제거된 노드는 delete를 통해 해제된다.
     */
    void RemoveNode(int32 NodeID);

    /** @brief ID로 그래프 내의 핀을 탐색 */
    UEdGraphPin* FindPin(int32 PinID);

    /** @brief 그래프의 노드에 고유한 ID를 발급한다. */
    int32 GetNextUniqueID();

private:
    int32 UniqueIDCounter = 1;
};
