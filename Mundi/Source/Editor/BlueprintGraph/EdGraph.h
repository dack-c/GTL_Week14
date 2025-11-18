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

    void Serialize(const bool bInIsLoading, JSON& InOutHandle) override;

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
    void RemoveNode(uint32 NodeID);

    /** @brief ID로 그래프 내의 핀을 탐색 (사용을 권장하지 않음) */
    UEdGraphPin* FindPin(uint32 PinID) const;

    /** @brief ID로 그래프 내의 노드를 탐색 */
    UEdGraphNode* FindNode(uint32 InNodeID) const;

    /** @brief 그래프의 노드에 고유한 ID를 발급한다. */
    int32 GetNextUniqueID();

private:
    int32 UniqueIDCounter = 1;
};
