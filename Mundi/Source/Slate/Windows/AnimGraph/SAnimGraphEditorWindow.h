#pragma once
#include "Windows/SWindow.h"
#include "imgui-node-editor/imgui_node_editor.h"
#include "imgui-node-editor/utilities/builders.h"
#include "imgui-node-editor/utilities/widgets.h"

namespace ed = ax::NodeEditor;
namespace util = ax::NodeEditor::Utilities;

enum class EPinType
{
    Flow,
    Bool,
    Int,
    Float,
    Object
};

struct FAnimGraphPin
{
    int32 PinID;
    int32 NodeID;
    bool bIsInput;
    FString Name;
    EPinType Type;

    FAnimGraphPin()
        : PinID(0)
        , NodeID(0)
        , bIsInput(false)
        , Name("None")
        , Type(EPinType::Flow)
    {}

    FAnimGraphPin(int32 InPinID, int32 InNodeID, bool bInIsInput, const FString& InName, EPinType InType)
        : PinID(InPinID)
        , NodeID(InNodeID)
        , bIsInput(bInIsInput)
        , Name(InName)
        , Type(InType)
    {}
};

struct FAnimGraphNode
{
    int32 NodeID;
    FString Title;
    ImVec2 Position;
    bool bIsPositionSet = false;
    ImColor Color;

    TArray<FAnimGraphPin> InputPins;
    TArray<FAnimGraphPin> OutputPins;

    FAnimGraphNode() : Color(255, 255, 255, 255) {}
};

struct FAnimGraphLink
{
    int32 LinkID;
    int32 StartPinID;
    int32 EndPinID;
};

struct FAnimGraphState
{
    FString Name;
    TArray<FAnimGraphNode> Nodes;
    TArray<FAnimGraphLink> Links;

    FAnimGraphNode* FindNode(int32 NodeID)
    {
        for (FAnimGraphNode& Node : Nodes)
        {
            if (Node.NodeID == NodeID) return &Node;
        }
        return nullptr;
    }

    FAnimGraphPin* FindPin(int32 PinID)
    {
        for (FAnimGraphNode& Node : Nodes)
        {
            for (FAnimGraphPin& Pin : Node.InputPins)
            {
                if (Pin.PinID == PinID) return &Pin;
            }
            for (FAnimGraphPin& Pin : Node.OutputPins)
            {
                if (Pin.PinID == PinID) return &Pin;
            }
        }
        return nullptr;
    }
};

/**
 * @brief 애니메이션 블루프린트 에디터 윈도우
 * - imgui-node-editor를 사용하여 애님 그래프를 시각화하고 편집한다.
 */
class SAnimGraphEditorWindow : public SWindow
{
public:
    SAnimGraphEditorWindow();
    virtual ~SAnimGraphEditorWindow();

    bool Initialize();

    virtual void OnRender() override;
    virtual void OnUpdate(float DeltaSeconds) override;

    bool IsOpen() const { return bIsOpen; }
    void Close() { bIsOpen = false; }

private:
    void RenderMenuBar();
    void RenderGraphEditor();
    void RenderContextMenu();

    int32 GetNextUniqueID() { return UniqueIDCounter++; }

    FAnimGraphNode* CreateNode(FAnimGraphState* Graph, const FString& Title, ImVec2 Position, ImColor Color = ImColor(255, 255, 255));
    void AddPin(FAnimGraphNode& Node, bool bIsInput, const FString& Name, EPinType Type);

    void DrawPinIcon(const FAnimGraphPin& Pin, bool bIsConnected, int32 AdjustAlpha);
    ImColor GetIconColor(EPinType Type);
    
private:
    bool IsPinLinked(int32 PinID);
    
    bool bIsOpen = true;

    ed::EditorContext* NodeEditorContext = nullptr;

    TArray<FAnimGraphState*> Tabs;
    FAnimGraphState* ActiveGraph = nullptr;

    int32 UniqueIDCounter = 1;

    UTexture* HeaderBackground = nullptr;
    util::BlueprintNodeBuilder* Builder = nullptr;
};