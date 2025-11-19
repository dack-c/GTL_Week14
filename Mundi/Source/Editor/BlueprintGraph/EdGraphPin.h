#pragma once
#include "EdGraphTypes.h"

class UEdGraphNode;

namespace FEdGraphPinCategory
{
    static const FName Exec("exec");
    static const FName Float("float");
    static const FName Int("int");
    static const FName Bool("bool");
    static const FName AnimSequence("UAnimSequence");
    static const FName BlendSpace1D("UBlendSpace1D");

    // AnimSequence와 BlendSpace1D는 모두 IAnimPoseProvider를 구현하므로 호환됨
    static const FName AnimPose("AnimPose");  // 공통 타입 (향후 확장용)
}

/**
 * @todo NodeID와 노드가 발급해주는 고유ID를 조합해서 고유한 PinID를 생성하는 방법을 생각해봅시다.
 */
UCLASS(DisplayName="UEdGraphPin", Description="블루프린트 그래프 핀")
class UEdGraphPin : public UObject
{
    DECLARE_CLASS(UEdGraphPin, UObject)
    
public:
    /** @note UObject UUID와 동일한 값 사용 */
    uint32 PinID;
    FString PinName;
    FEdGraphPinType PinType;
    EEdGraphPinDirection Direction;

    UEdGraphNode* OwningNode = nullptr;

    /** @brief 이 핀이 연결된 다른 핀들의 목록 (UE5 FAnimGraphLink) */
    TArray<UEdGraphPin*> LinkedTo;

    FString DefaultValue;

public:
    /** @note RTTI 지원용 디폴트 생성자 */
    UEdGraphPin() : PinID(UUID) {} 
    
    UEdGraphPin(UEdGraphNode* InOwner, EEdGraphPinDirection InDir, FName InCategory, const FString& InName, const FString& InDefaultValue)
        : PinID(UUID)
        , PinName(InName)
        , PinType(InCategory)
        , Direction(InDir)
        , OwningNode(InOwner)
        , DefaultValue(InDefaultValue)
    {}

    virtual void Serialize(const bool bInIsLoading, JSON& InOutHandle) override;

    void MakeLinkTo(UEdGraphPin* ToPin)
    {
        if (ToPin && Direction == EEdGraphPinDirection::EGPD_Output && ToPin->Direction == EEdGraphPinDirection::EGPD_Input)
        {
            LinkedTo.AddUnique(ToPin);
            ToPin->LinkedTo.AddUnique(this);
        }
    }

    void BreakLinkTo(UEdGraphPin* ToPin)
    {
        LinkedTo.Remove(ToPin);
        ToPin->LinkedTo.Remove(this);
    }
    
    void BreakAllLinks()
    {
        for (UEdGraphPin* OtherPin : LinkedTo)
        {
            OtherPin->LinkedTo.Remove(this);
        }
        LinkedTo.Empty();
    }
};
