#pragma once
#include <variant>

/** @note UE5 EEdGraphPinDirection */
enum class EEdGraphPinDirection
{
    EGPD_Input,
    EGPD_Output,
    EGPD_MAX
};

/** @note UE5 FEdGraphPinType */
struct FEdGraphPinType
{
    FName PinCategory; // "exec", "float", "int", "bool", "object", "none"
    
    /* 언리얼 엔진과의 일관성을 위한 변수들 (옵션) */
    FName PinSubCategory;                    
    UObject* PinSubCategoryObject = nullptr;

    FEdGraphPinType(FName InCategory = FName()/* = NAME_None */) : PinCategory(InCategory) {}
};

// ----------------------------------------------------------------
//	블루프린트 표현식(Expression)용 구조체
// ----------------------------------------------------------------

/**
 * @brief 블루프린트 표현식 타입용 std::varaint
 * @note 블루프린트 표현식에서 사용되는 모든 타입은 이곳에 포함되어야 한다.
 * @todo 현재는 구현의 용이함을 위해 std::variant를 사용하지만, OCP(Open-Close Principle)원칙 위배이다.
 *       나중에 여유가 된다면 Type Erasure 등으로 리팩터링한다.
 */
using FBlueprintValueType = std::variant<
    int32,
    float,
    bool
>;

/**
 * @brief 블루프린트 표현식 값 저장용 구조체 
 */
struct FBlueprintValue
{
    FBlueprintValue() : Value(0) {}
    
    template<typename T>
    FBlueprintValue(T InValue) : Value(InValue) {}

    /** @brief 타입 안전하게 값을 가져온다.  */
    template<typename TValue>
    TValue Get() const
    {
        // 타입 안정성(Type Safety)용 assert (암시적 형변환 허용하지 않음)
        assert(std::holds_alternative<TValue>(Value));
        
        if (std::holds_alternative<TValue>(Value))
        {
            return std::get<TValue>(Value);
        }

        // @todo assert가 활성화되지 않았을 경우 디폴트 값 반환 (안전한지 확인 필요)
        return TValue{};
    }

    /** @brief 유효성 체크 */
    bool IsValid() const { return Value.index() != std::variant_npos; }

    FBlueprintValueType Value;
};