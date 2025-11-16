#include "pch.h"
#include "K2Node_LiteralNodes.h"

#include "BlueprintActionDatabase.h"

// ----------------------------------------------------------------
//	[Int] 리터럴 노드
// ----------------------------------------------------------------

IMPLEMENT_CLASS(UK2Node_Literal_Int, UK2Node)

void UK2Node_Literal_Int::AllocateDefaultPins()
{
    CreatePin(EEdGraphPinDirection::EGPD_Output, FEdGraphPinCategory::Int, "Value");

    RenderBody = [](UEdGraphNode* Self)
    {
        auto Node = Cast<UK2Node_Literal_Int>(Self);
        if (Node)
        {
            ImGui::PushItemWidth(100.0f);
            ImGui::DragInt("##value", &Node->Value, 0.1f);
            ImGui::PopItemWidth();
        }
    };
}

void UK2Node_Literal_Int::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(GetClass());

    Spawner->MenuName = GetNodeTitle();
    Spawner->Category = GetMenuCategory();

    ActionRegistrar.AddAction(Spawner);
}

// ----------------------------------------------------------------
//	[Float] 리터럴 노드
// ----------------------------------------------------------------

IMPLEMENT_CLASS(UK2Node_Literal_Float, UK2Node)

void UK2Node_Literal_Float::AllocateDefaultPins()
{
    CreatePin(EEdGraphPinDirection::EGPD_Output, FEdGraphPinCategory::Float, "Value");

    RenderBody = [](UEdGraphNode* Self)
    {
        /** @todo 아직 UPROPERTY 시스템에 익숙하지 않아서, 일단 Cast로 처리함 */
        auto Node = Cast<UK2Node_Literal_Float>(Self);
        if (Node)
        {
            ImGui::PushItemWidth(100.0f);
            ImGui::DragFloat("##value", &Node->Value, 0.01f);
            ImGui::PopItemWidth();
        }
    };
}

void UK2Node_Literal_Float::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(GetClass());

    Spawner->MenuName = GetNodeTitle();
    Spawner->Category = GetMenuCategory();

    ActionRegistrar.AddAction(Spawner);
}
