#include "pch.h"
#include "AnimSequenceBase.h"
#include "ObjectFactory.h"

IMPLEMENT_CLASS(UAnimSequenceBase)

UAnimSequenceBase::UAnimSequenceBase()
    : RateScale(1.0f)
    , bLoop(true)
    , DataModel(nullptr)
{
    // Create DataModel
    DataModel = NewObject<UAnimDataModel>();
}

UAnimDataModel* UAnimSequenceBase::GetDataModel() const
{
    return DataModel;
}