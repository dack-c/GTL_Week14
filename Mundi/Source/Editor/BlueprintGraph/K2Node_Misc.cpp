#include "pch.h"
#include "ImGui/imgui_stdlib.h"
#include "imgui-node-editor/imgui_node_editor.h"
#include "K2Node_Misc.h"
#include "BlueprintActionDatabase.h"

// ----------------------------------------------------------------
//	[Input] 키보드 입력 확인 노드
// ----------------------------------------------------------------

IMPLEMENT_CLASS(UK2Node_IsPressed, UK2Node)

void UK2Node_IsPressed::AllocateDefaultPins()
{
    CreatePin(EEdGraphPinDirection::EGPD_Output, FEdGraphPinCategory::Bool, "Result");
    
    TitleColor = ImColor(220, 48, 48);
}

void UK2Node_IsPressed::RenderBody()
{

    ImGui::PushItemWidth(150.0f);
    ImGui::InputText("키 이름", &KeyName);
    ImGui::PopItemWidth();
}

void UK2Node_IsPressed::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(GetClass());

    Spawner->MenuName = GetNodeTitle();
    Spawner->Category = GetMenuCategory();

    ActionRegistrar.AddAction(Spawner);
}

// ----------------------------------------------------------------
//	[Input] 마우스 위치 확인 노드
// ----------------------------------------------------------------

IMPLEMENT_CLASS(UK2Node_GetMousePosition, UK2Node)

void UK2Node_GetMousePosition::AllocateDefaultPins()
{
    CreatePin(EEdGraphPinDirection::EGPD_Output, FEdGraphPinCategory::Float, "X");
    CreatePin(EEdGraphPinDirection::EGPD_Output, FEdGraphPinCategory::Float, "Y");

    TitleColor = ImColor(147, 226, 74);
}

void UK2Node_GetMousePosition::RenderBody()
{
}

void UK2Node_GetMousePosition::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(GetClass());

    Spawner->MenuName = GetNodeTitle();
    Spawner->Category = GetMenuCategory();

    ActionRegistrar.AddAction(Spawner);
}