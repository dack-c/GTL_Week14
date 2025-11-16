#pragma once

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