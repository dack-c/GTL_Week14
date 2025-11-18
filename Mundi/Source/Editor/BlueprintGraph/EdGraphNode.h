#pragma once

#include "BlueprintTypes.h"
#include "EdGraphPin.h"

UCLASS(DisplayName="UEdGraphNode", Description="블루프린트 노드 베이스")
class UEdGraphNode : public UObject
{
    DECLARE_CLASS(UEdGraphNode, UObject)
    
public:
    /** @brief 노드가 소유한 모든 핀의 목록 */
    TArray<UEdGraphPin*> Pins;

    /** @brief 에디터 내 노드 X 위치 */
    float NodePosX = 0.0f;
    
    /** @brief 에디터 내 노드 Y 위치 */
    float NodePosY = 0.0f;

    /** @brief ImGui에서 사용될 위치 */
    ImVec2 Position;
    
    /** @brief SWindow에서 초기 등록에 사용되는 변수 */
    bool bIsPositionSet = false;

    /** @brief 노드 제목 색상 */
    ImColor TitleColor = ImColor(255, 255, 255);

    /** @note UEdGraph가 발급해주는 ID 사용 */
    uint32 NodeID;

public:
    UEdGraphNode() = default;
    
    virtual ~UEdGraphNode()
    {
        for (UEdGraphPin* Pin : Pins)
        {
            DeleteObject(Pin);
        }
        Pins.Empty();
    }

    void Serialize(const bool bInIsLoading, JSON& InOutHandle) override;

    /** @brief 노드 타입에 필요한 기본 핀들을 생성한다. */
    virtual void AllocateDefaultPins() {}

    /** @brief 노드 본문의 커스텀 UI를 렌더링한다. */
    virtual void RenderBody() {}

    /** @brief 노드 제목을 반환한다. */
    virtual FString GetNodeTitle() const { return "Base Node"; }

    /** @brief 특정 핀에 대한 값을 평가해서 반환한다(블루프린트 표현식 용). */
    virtual FBlueprintValue EvaluatePin(const UEdGraphPin* OutputPin, FBlueprintContext* Context) { return FBlueprintValue{}; }

    /** @brief UUID를 PinID로 활용해서 핀을 생성한다. */
    UEdGraphPin* CreatePin(EEdGraphPinDirection Dir, FName PinCategory, const FString& PinName, const FString& DefaultValue = "")
    {
        UEdGraphPin* NewPin = new UEdGraphPin(this, Dir, PinCategory, PinName, DefaultValue);
        Pins.Add(NewPin);
        return NewPin;
    }

    /** @brief 이름으로 핀을 찾는다. */
    UEdGraphPin* FindPin(const FString& PinName, EEdGraphPinDirection OptionalDirection = EEdGraphPinDirection::EGPD_MAX) const
    {
        for (UEdGraphPin* Pin : Pins)
        {
            if (Pin->PinName == PinName && (OptionalDirection == EEdGraphPinDirection::EGPD_MAX || Pin->Direction == OptionalDirection))
            {
                return Pin;
            }
        }
        UE_LOG("UEdGraphPin::FindPin: 핀 '%s'를 찾는데 실패했습니다.", PinName.c_str());
        return nullptr;
    }
};
