#include "pch.h" 
#include "SAnimGraphEditorWindow.h"

static inline ed::NodeId ToNodeId(int32 id) { return (uintptr_t)id; }
static inline ed::PinId ToPinId(int32 id) { return (uintptr_t)id; }
static inline ed::LinkId ToLinkId(int32 id) { return (uintptr_t)id; }

SAnimGraphEditorWindow::SAnimGraphEditorWindow()
{
    NodeEditorContext = nullptr;
    ActiveGraph = nullptr;
    UniqueIDCounter = 1;
    bIsOpen = true;
    HeaderBackground = nullptr;
    Builder = nullptr;
}

bool SAnimGraphEditorWindow::Initialize()
{
    ed::Config Config;
    NodeEditorContext = ed::CreateEditor(&Config);

    HeaderBackground = RESOURCE.Load<UTexture>("Data/Icon/BlueprintBackground.png");

    int32 TexWidth = HeaderBackground ? HeaderBackground->GetWidth() : 0;
    int32 TexHeight = HeaderBackground ? HeaderBackground->GetHeight() : 0;
    Builder = new util::BlueprintNodeBuilder((ImTextureID)HeaderBackground->GetShaderResourceView(), TexWidth, TexHeight);

    FAnimGraphState* AnimGraph = new FAnimGraphState();
    AnimGraph->Name = "Anim Graph";
    
    Tabs.Add(AnimGraph);
    ActiveGraph = AnimGraph;

    return true;
}

SAnimGraphEditorWindow::~SAnimGraphEditorWindow()
{
    for (FAnimGraphState* Graph : Tabs)
    {
        delete Graph;
    }
    Tabs.Empty();

    if (NodeEditorContext)
    {
        ed::DestroyEditor(NodeEditorContext);
        NodeEditorContext = nullptr;
    }

    if (Builder)
    {
        delete Builder;
        Builder = nullptr;
    }
}

void SAnimGraphEditorWindow::OnUpdate(float DeltaSeconds)
{
    // (필요시 그래프 상태 업데이트 로직 추가)
}

void SAnimGraphEditorWindow::OnRender()
{
    if (!bIsOpen) return;

    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Animation Blueprint Editor", &bIsOpen))
    {
        RenderMenuBar();

        // (TODO: 탭 바 로직은 SSkeletalMeshViewerWindow를 참고하여 추가)
        // ...
        // 여기서는 단일 탭(ActiveGraph)만 있다고 가정
        // ...

        RenderGraphEditor();
    }
    ImGui::End();
}

void SAnimGraphEditorWindow::RenderMenuBar()
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save")) { /* TODO: 저장 로직 */ }
            if (ImGui::MenuItem("Load")) { /* TODO: 로드 로직 */ }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

void SAnimGraphEditorWindow::RenderGraphEditor()
{
    if (!ActiveGraph) return;

    ed::SetCurrentEditor(NodeEditorContext);
    ed::Begin("AnimGraphEditor");

    for (FAnimGraphNode& Node : ActiveGraph->Nodes)
    {
        if (!Node.bIsPositionSet)
        {
            ed::SetNodePosition(ToNodeId(Node.NodeID), Node.Position);
            Node.bIsPositionSet = true;
        }

        Builder->Begin(ToNodeId(Node.NodeID));

        Builder->Header(Node.Color);
        {
            ImGui::TextUnformatted(Node.Title.c_str());

            ImGui::SameLine();
            ImGui::Dummy(ImVec2(0, 28));
        }
        Builder->EndHeader();

        for (FAnimGraphPin& Pin : Node.InputPins)
        {
            Builder->Input(ToPinId(Pin.PinID));

            DrawPinIcon(Pin, IsPinLinked(Pin.PinID), 255);

            ImGui::SameLine();
            if (!Pin.Name.empty())
            {
                ImGui::TextUnformatted(Pin.Name.c_str());
            }
            Builder->EndInput();
        }

        Builder->Middle();

        for (FAnimGraphPin& Pin : Node.OutputPins)
        {
            Builder->Output(ToPinId(Pin.PinID));

            if (!Pin.Name.empty())
            {
                ImGui::TextUnformatted(Pin.Name.c_str());
                ImGui::SameLine();
            }
            
            DrawPinIcon(Pin, IsPinLinked(Pin.PinID), 255);

            Builder->EndOutput();
        }

        Builder->End();

        Node.Position = ed::GetNodePosition(ToNodeId(Node.NodeID));
    }

    for (const FAnimGraphLink& Link : ActiveGraph->Links)
    {
        ed::Link(ToLinkId(Link.LinkID), ToPinId(Link.StartPinID), ToPinId(Link.EndPinID));
    }

    if (ed::BeginCreate())
    {
        ed::PinId StartPinID_UI, EndPinID_UI;
        if (ed::QueryNewLink(&StartPinID_UI, &EndPinID_UI))
        {
            int32 StartPinID = (int32)(uintptr_t)StartPinID_UI;
            int32 EndPinID = (int32)(uintptr_t)EndPinID_UI;

            // TODO: 핀 타입 유효성 검사 (FindPin(StartPinID)->Type == FindPin(EndPinID)->Type)
            
            if (ed::AcceptNewItem())
            {
                FAnimGraphLink NewLink;
                NewLink.LinkID = GetNextUniqueID();
                NewLink.StartPinID = StartPinID;
                NewLink.EndPinID = EndPinID;
                ActiveGraph->Links.Add(NewLink);
            }
        }
    }
    ed::EndCreate();

    if (ed::BeginDelete())
    {
        ed::LinkId DestroyedLinkID_UI;
        while (ed::QueryDeletedLink(&DestroyedLinkID_UI))
        {
            if (ed::AcceptDeletedItem())
            {
                int32 DestroyedLinkID = (int32)(uintptr_t)DestroyedLinkID_UI;
                for (int i = ActiveGraph->Links.Num() - 1; i >= 0; --i)
                {
                    if (ActiveGraph->Links[i].LinkID == DestroyedLinkID)
                    {
                        ActiveGraph->Links.RemoveAt(i);
                        break;
                    }
                }
            }
        }
    }
    ed::EndDelete();

    ed::Suspend();
    RenderContextMenu();
    ed::Resume();

    ed::End();
    ed::SetCurrentEditor(nullptr);
}

void SAnimGraphEditorWindow::RenderContextMenu()
{
    if (ed::ShowBackgroundContextMenu())
    {
        ImGui::OpenPopup("NodeContextMenu");
    }

    if (ImGui::BeginPopup("NodeContextMenu"))
    {
        ImVec2 ClickPos = ed::ScreenToCanvas(ImGui::GetMousePosOnOpeningCurrentPopup()); // 캔버스 좌표계 기준 클릭 위치
        
        if (ImGui::MenuItem("Add 'Play Idle' Node"))
        {
            FAnimGraphNode* Node = CreateNode(ActiveGraph, "Play Idle", ClickPos);
            AddPin(*Node, false, "Pose", EPinType::Flow); // 출력 핀
        }
        
        if (ImGui::MenuItem("Add 'Blend (A/B)' Node"))
        {
            FAnimGraphNode* Node = CreateNode(ActiveGraph, "Blend (A/B)", ClickPos);
            AddPin(*Node, true, "Pose A", EPinType::Flow);
            AddPin(*Node, true, "Pose B", EPinType::Flow);
            AddPin(*Node, true, "Alpha", EPinType::Flow);
            AddPin(*Node, false, "Pose", EPinType::Flow);
        }
        
        if (ImGui::MenuItem("Add 'State Machine'"))
        {
            CreateNode(ActiveGraph, "Locomotion", ClickPos);
            // TODO: (상태 머신은 더블클릭 시 다른 탭으로 이동하는 로직 필요)
        }

        ImGui::EndPopup();
    }
}

FAnimGraphNode* SAnimGraphEditorWindow::CreateNode(FAnimGraphState* Graph, const FString& Title, ImVec2 Position, ImColor Color)
{
    FAnimGraphNode NewNode;
    NewNode.NodeID = GetNextUniqueID();
    NewNode.Title = Title;
    NewNode.Position = Position; 
    NewNode.bIsPositionSet = false;
    NewNode.Color = Color;
    Graph->Nodes.Add(NewNode);

    return &Graph->Nodes.Last(); 
}

void SAnimGraphEditorWindow::AddPin(FAnimGraphNode& Node, bool bIsInput, const FString& Name, EPinType PinType)
{
    FAnimGraphPin NewPin(GetNextUniqueID(), Node.NodeID, bIsInput, Name, PinType);
    
    if (bIsInput)
    {
        Node.InputPins.Add(NewPin);
    }
    else
    {
        Node.OutputPins.Add(NewPin);
    }
}

ImColor SAnimGraphEditorWindow::GetIconColor(EPinType Type)
{
    switch (Type)
    {
    default:
    case EPinType::Flow:   return ImColor(255, 255, 255);
    case EPinType::Bool:   return ImColor(220,  48,  48);
    case EPinType::Int:    return ImColor( 68, 201, 156);
    case EPinType::Float:  return ImColor(147, 226,  74);
    case EPinType::Object: return ImColor( 51, 150, 215);
    }
}

bool SAnimGraphEditorWindow::IsPinLinked(int32 PinID)
{
    if (!ActiveGraph || PinID == 0)
        return false;

    for (const FAnimGraphLink& Link : ActiveGraph->Links)
    {
        if (Link.StartPinID == PinID || Link.EndPinID == PinID)
        {
            return true;
        }
    }

    return false;
}

void SAnimGraphEditorWindow::DrawPinIcon(const FAnimGraphPin& Pin, bool bIsConnected, int32 Alpha)
{
    ax::Drawing::IconType IconType;
    ImColor Color = GetIconColor(Pin.Type);
    Color.Value.w = Alpha / 255.0f; 

    switch (Pin.Type)
    {
    case EPinType::Flow:   IconType = ax::Drawing::IconType::Flow;   break;
    case EPinType::Bool:   IconType = ax::Drawing::IconType::Circle; break;
    case EPinType::Int:    IconType = ax::Drawing::IconType::Circle; break;
    case EPinType::Float:  IconType = ax::Drawing::IconType::Circle; break;
    case EPinType::Object: IconType = ax::Drawing::IconType::Square; break; 
    default: return;
    }

    ax::Widgets::Icon(ImVec2(24, 24), IconType, bIsConnected, Color, ImColor(32, 32, 32, Alpha));
}
