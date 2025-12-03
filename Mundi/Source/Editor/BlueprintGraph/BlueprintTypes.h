#pragma once

#include <variant>

// Forward declarations
class UAnimSequence;
class UBlendSpace1D;
class UBlendSpace2D;

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
    bool,
    UAnimSequence*,
    UBlendSpace1D*,
    UBlendSpace2D*
>;

/**
 * @brief 블루프린트 표현식 값 저장용 구조체 
 */
struct FBlueprintValue
{
    FBlueprintValue() : Value(0) {}

    template<typename T>
    FBlueprintValue(T InValue) : Value(InValue) {}

    // nullptr 처리용 생성자 - UAnimSequence*로 기본 변환
    FBlueprintValue(std::nullptr_t) : Value(static_cast<UAnimSequence*>(nullptr)) {}

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

/**
 * @brief 블루프린트 표현식 평가 시 전달되는 실행 컨텍스트
 * 언리얼엔진의 FFrame이나 FAnimationUpdateContext와 유사한 역할
 *
 * @todo RTTI 시스템이 완벽히 구비되지 않아서, 컨텍스트 메뉴에서는 현재 컨텍스트와 무관한 노드를 사용가능하다.
 *       이를 제한할 방법을 떠올리거나, 사용자가 잘못 사용하지 않도록 UI/UX를 개선한다.
 */
struct FBlueprintContext
{
    /** @brief 현재 그래프를 실행하고 있는 주체 (e.g., PlayerController, AnimInstance, etc) */
    UObject* SourceObject;

    FBlueprintContext(UObject* InSourceObject) : SourceObject(InSourceObject) {}
};