#pragma once

#include "Windows/SWindow.h"
#include "imgui-node-editor/imgui_node_editor.h"
#include "imgui-node-editor/utilities/builders.h"
#include "imgui-node-editor/utilities/widgets.h"

class UEdGraph;
class UEdGraphNode;
class UEdGraphPin;
struct FEdGraphPinType;

namespace ed = ax::NodeEditor;
namespace util = ax::NodeEditor::Utilities;

/**
 * @brief 범용 블루프린트 그래프 에디터 윈도우
 * 
 * @note UEdGraph 에셋을 시각화하고 편집한다.
 */
class SGraphEditorWindow : public SWindow
{
public:
    /** @note 각 노드의 ID가 32-bit라고 가정하고, 두 비트를 이어붙여서 고유한 링크 ID를 생성한다. */
    using LinkId = uint64;
    
    SGraphEditorWindow();
    virtual ~SGraphEditorWindow();

    // ----------------------------------------------------------------
    //	SWindow 인터페이스
    // ----------------------------------------------------------------
public:
    virtual void OnRender() override;
    virtual void OnUpdate(float DeltaSeconds) override;
    
    // ----------------------------------------------------------------
    //	SGraphEditorWindow 인터페이스
    // ----------------------------------------------------------------
public:    
    /** @brief 에디터를 초기화하고 편집할 그래프를 설정한다. */
    bool Initialize(UEdGraph* InGraphToEdit);

    /** @brief 창이 열려있는지 확인한다. */
    bool IsOpen() const { return bIsOpen; }

    /** @brief 창을 닫는다. */
    void Close() { bIsOpen = false; }

private:
    /** @brief 메뉴바 렌더링 */
    void RenderMenuBar();

    /** @brief 노드 에디터 UI 전체 렌더링 */
    void RenderEditor();

    /** @brief 단일 노드 렌더링 (핀 포함) */
    void RenderNode(UEdGraphNode* Node);

    /** @brief 단일 핀 렌더링 (입/출력, 아이콘, 텍스트) */
    void RenderPin(UEdGraphPin* Pin);

    /** @brief 그래프의 모든 링크 렌더링 */
    void RenderLinks();

    /** @brief 링크 생성 처리 */
    void HandleCreation();

    /** @brief 노드/링크 삭제 처리 */
    void HandleDeletion();

    /** @brief 우클릭 컨텍스트 메뉴 렌더링 (노드 스폰) */
    void RenderContextMenu();

    /** @brief 핀 타입에 맞는 색상 반환 */
    ImColor GetPinColor(const FEdGraphPinType& PinType);

    /** @brief 핀 아이콘 렌더링 */
    void DrawPinIcon(const UEdGraphPin* Pin, bool bIsConnected, int32 Alpha);

    /** @brief 고유한 링크 ID 생성 */
    LinkId GetLinkId(UEdGraphPin* StartPin, UEdGraphPin* EndPin) const;

private:
    /** @brief imgui-node-editor 컨텍스트 */
    ed::EditorContext* Context = nullptr;

    /** @brief 현재 편집 중인 그래프 */
    UEdGraph* Graph = nullptr;

    /** @brief 노드 UI 빌더 (헤더 이미지 등) */
    util::BlueprintNodeBuilder* Builder = nullptr;

    /** @brief 노드 헤더 배경 텍스쳐 */
    UTexture* HeaderBackground = nullptr;

    /** @brief 창 열림 여부 */
    bool bIsOpen = false;
};
