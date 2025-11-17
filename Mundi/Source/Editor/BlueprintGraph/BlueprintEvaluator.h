#pragma once

#include "EdGraphNode.h"
#include "EdGraphPin.h"

/**
 * @brief 블루프린트 표현식 평가용 헬퍼함수를 포함하는 클래스
 */
class FBlueprintEvaluator
{
public:
    /**
     * @brief 입력 핀으로 들어오는 값을 평가한다.
     * 1. 연결된 핀이 있으면 EvaluatePin을 요청한다.
     * 2. 연결된 핀이 없으면 DefaultValue를 파싱해서 반환한다.
     * @todo 연결이 없을 경우 처리를 각 클래스별로 구현한다 (현재는 제대로 작동하지 않는다).
     */
    template <typename T>
    static T EvaluateInput(const UEdGraphPin* InputPin, FBlueprintContext* Context)
    {
        if (!InputPin)
        {
            return T{};
        }

        if (InputPin->LinkedTo.Num() > 0)
        {
            // @note 표현식용 입력핀은 하나의 Source만을 가져야 한다.
            UEdGraphPin* SourcePin = InputPin->LinkedTo[0];

            if (SourcePin && SourcePin->OwningNode)
            {
                FBlueprintValue Result = SourcePin->OwningNode->EvaluatePin(SourcePin, Context);
                return Result.Get<T>();
            }
        }

        return ParseString<T>(InputPin->DefaultValue);
    }

private:
    /**
     * @brief DefaultValue 파싱용 함수
     * @todo 타입별로 특수화 필요
     */
    template <typename T>
    static T ParseString(const FString& InString) { return T{}; }
};

template<>
inline int32 FBlueprintEvaluator::ParseString<int32>(const FString& InString) { return std::stoi(InString); }

template<>
inline float FBlueprintEvaluator::ParseString<float>(const FString& InString) { return std::stof(InString); }

template <>
inline bool FBlueprintEvaluator::ParseString<bool>(const FString& InString) { return InString == "true"; }