#include "pch.h"
#include "EdGraph.h"
#include "EdGraphNode.h"

IMPLEMENT_CLASS(UEdGraph, UObject)

UEdGraph::~UEdGraph()
{
    for (UEdGraphNode* Node : Nodes)
    {
        DeleteObject(Node);
    }
    Nodes.Empty();
}

UEdGraphNode* UEdGraph::AddNode(UEdGraphNode* Node)
{
    Nodes.Add(Node);
    return Node;
}

void UEdGraph::RemoveNode(UEdGraphNode* Node)
{
    if (Nodes.Contains(Node))
    {
        for (UEdGraphPin* Pin : Node->Pins)
        {
            Pin->BreakAllLinks(); 
        }

        Nodes.Remove(Node);
        DeleteObject(Node);
    }
}

void UEdGraph::RemoveNode(int32 NodeID)
{
    for (UEdGraphNode* Node : Nodes)
    {
        if (Node->NodeID == NodeID)
        {
            for (UEdGraphPin* Pin : Node->Pins)
            {
                Pin->BreakAllLinks();
            }
        }
        Nodes.Remove(Node);
        DeleteObject(Node);

        break;
    }
}

UEdGraphPin* UEdGraph::FindPin(int32 PinID)
{
    for (UEdGraphNode* Node : Nodes)
    {
        for (UEdGraphPin* Pin : Node->Pins)
        {
            if (Pin->PinID == PinID)
            {
                return Pin;
            }
        }
    }
    return nullptr;
}

int32 UEdGraph::GetNextUniqueID()
{
    return UniqueIDCounter++;
}
