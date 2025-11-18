#include "pch.h"
#include "ImGui/imgui_stdlib.h"
#include "imgui-node-editor/imgui_node_editor.h"
#include "K2Node_Animation.h"
#include "BlueprintActionDatabase.h"

namespace ed = ax::NodeEditor;

// ----------------------------------------------------------------
//	[AnimSequence] 애니메이션 시퀀스 노드 
// ----------------------------------------------------------------

IMPLEMENT_CLASS(UK2Node_AnimSequence, UK2Node)

UK2Node_AnimSequence::UK2Node_AnimSequence()
{
    TitleColor = ImColor(100, 120, 255);
}

void UK2Node_AnimSequence::Serialize(const bool bInIsLoading, JSON& InOutHandle)
{
    UK2Node::Serialize(bInIsLoading, InOutHandle);

    if (bInIsLoading)
    {
        FString AnimPath;
        if (FJsonSerializer::ReadString(InOutHandle, "AnimPath", AnimPath))
        {
            if (!AnimPath.empty())
            {
                Value = RESOURCE.Get<UAnimSequence>(AnimPath);
            }
        }
    }
    else
    {
        if (Value)
        {
            InOutHandle["AnimPath"] = Value->GetFilePath();
        }
    }
}

void UK2Node_AnimSequence::AllocateDefaultPins()
{
    CreatePin(EEdGraphPinDirection::EGPD_Output, FEdGraphPinCategory::AnimSequence, "Value");
}

/**
 * @note ed::Suspend를 활용하지 않으면 스크린 공간이 아닌 캔버스 공간 좌표가 사용되어서
 * 정상적으로 창이 뜨지 않는 이슈 존재가 존재했었다. 현재는 해결되었다.
 */
void UK2Node_AnimSequence::RenderBody()
{
    FString PopupID = "##AnimSelectPopup_";
    PopupID += std::to_string(NodeID);

    FString PreviewName = "None";
    if (Value)
    {
        PreviewName = Value->GetFilePath();
    }

    if (ImGui::Button(PreviewName.c_str()))
    {
        ed::Suspend();
        ImGui::OpenPopup(PopupID.c_str());
        ed::Resume();
    }

    if (ImGui::IsPopupOpen(PopupID.c_str()))
    {
        ed::Suspend();
        
        if (ImGui::BeginPopup(PopupID.c_str()))
        {
            TArray<UAnimSequence*> AnimSequences = RESOURCE.GetAll<UAnimSequence>();

        for (UAnimSequence* Anim : AnimSequences)
        {
            if (!Anim) continue;

            const FString AssetName = Anim->GetFilePath();
            bool bIsSelected = (Value == Anim);
              
            if (ImGui::Selectable(AssetName.c_str(), bIsSelected))
            {
                Value = Anim; 
                Anim->LoadMeta(Anim->GetNotifyPath());
                ImGui::CloseCurrentPopup();
            }

            for (UAnimSequence* Anim : AnimSequences)
            {
                if (!Anim) continue;

                const FString AssetName = Anim->GetFilePath();
                bool bIsSelected = (Value == Anim);

                if (ImGui::Selectable(AssetName.c_str(), bIsSelected))
                {
                    Value = Anim;
                    ImGui::CloseCurrentPopup();
                }
                
                if (bIsSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndPopup();
        }
        
        ed::Resume();
    }
}

FBlueprintValue UK2Node_AnimSequence::EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context)
{
    if (OutputPin->PinName == "Value")
    {
        return Value;
    }
    return nullptr;
}

// ----------------------------------------------------------------
//	[AnimStateEntry] 애니메이션 상태 머신 진입점
// ----------------------------------------------------------------

IMPLEMENT_CLASS(UK2Node_AnimStateEntry, UK2Node)

UK2Node_AnimStateEntry::UK2Node_AnimStateEntry()
{
    TitleColor = ImColor(150, 150, 150);
}

void UK2Node_AnimStateEntry::AllocateDefaultPins()
{
    CreatePin(EEdGraphPinDirection::EGPD_Output, FEdGraphPinCategory::Exec, "Entry"); 
}

void UK2Node_AnimStateEntry::RenderBody()
{
}

void UK2Node_AnimStateEntry::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(GetClass());

    Spawner->MenuName = GetNodeTitle();
    Spawner->Category = GetMenuCategory();

    ActionRegistrar.AddAction(Spawner);
}

void UK2Node_AnimSequence::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(GetClass());

    Spawner->MenuName = GetNodeTitle();
    Spawner->Category = GetMenuCategory();

    ActionRegistrar.AddAction(Spawner);
}

// ----------------------------------------------------------------
//	[AnimState] 애니메이션 상태 노드
// ----------------------------------------------------------------

IMPLEMENT_CLASS(UK2Node_AnimState, UK2Node)

UK2Node_AnimState::UK2Node_AnimState()
{
    TitleColor = ImColor(200, 100, 100);
}

void UK2Node_AnimState::Serialize(const bool bInIsLoading, JSON& InOutHandle)
{
    UK2Node::Serialize(bInIsLoading, InOutHandle);

    if (bInIsLoading)
    {
        FJsonSerializer::ReadString(InOutHandle, "StateName", StateName);
    }
    else
    {
        InOutHandle["StateName"] = StateName;
    }
}

void UK2Node_AnimState::AllocateDefaultPins()
{
    // 상태 머신 그래프의 흐름(Flow)을 위한 Exec 핀
    CreatePin(EEdGraphPinDirection::EGPD_Input, FEdGraphPinCategory::Exec, "Enter");
    CreatePin(EEdGraphPinDirection::EGPD_Output, FEdGraphPinCategory::Exec, "Exit");

    // FAnimationState의 멤버에 해당하는 핀들
    CreatePin(EEdGraphPinDirection::EGPD_Input, FEdGraphPinCategory::AnimSequence, "Animation");
    CreatePin(EEdGraphPinDirection::EGPD_Input, FEdGraphPinCategory::Bool, "Looping", "false");
    CreatePin(EEdGraphPinDirection::EGPD_Input, FEdGraphPinCategory::Float, "PlayRate", "1.0");
}

void UK2Node_AnimState::RenderBody()
{
    ImGui::PushItemWidth(150.0f);
    ImGui::InputText("상태 이름", &StateName);
    ImGui::PopItemWidth();
}

void UK2Node_AnimState::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(GetClass());

    Spawner->MenuName = GetNodeTitle();
    Spawner->Category = GetMenuCategory();

    ActionRegistrar.AddAction(Spawner);
}

// ----------------------------------------------------------------
//	[AnimTransition] 애니메이션 전이 노드
// ----------------------------------------------------------------

IMPLEMENT_CLASS(UK2Node_AnimTransition, UK2Node)

UK2Node_AnimTransition::UK2Node_AnimTransition()
{
    TitleColor = ImColor(100, 100, 200);
}

void UK2Node_AnimTransition::AllocateDefaultPins()
{
    // 상태 머신 그래프의 흐름(Flow)을 위한 Exec 핀
    CreatePin(EEdGraphPinDirection::EGPD_Input, FEdGraphPinCategory::Exec, "Execute"); 
    CreatePin(EEdGraphPinDirection::EGPD_Output, FEdGraphPinCategory::Exec, "Transition To");
    
    // FStateTransition의 멤버에 해당하는 핀들
    CreatePin(EEdGraphPinDirection::EGPD_Input, FEdGraphPinCategory::Bool, "Can Transition");
    CreatePin(EEdGraphPinDirection::EGPD_Input, FEdGraphPinCategory::Float, "Blend Time");
}

void UK2Node_AnimTransition::RenderBody()
{
}

void UK2Node_AnimTransition::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(GetClass());

    Spawner->MenuName = GetNodeTitle();
    Spawner->Category = GetMenuCategory();

    ActionRegistrar.AddAction(Spawner);
}