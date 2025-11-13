#pragma once
#include "AnimSequenceBase.h"


class UAnimSequence : public UAnimSequenceBase
{
    DECLARE_CLASS(UAnimSequence, UAnimSequenceBase)

public:
    UAnimSequence();
    virtual ~UAnimSequence()  = default;

protected:
    
    TArray<FAnimNotifyEvent> Notifies;

    //TArray<FAnimNotifyTrack> AnimNotifyTracks;

    UAnimDataModel* DataModel;

    float RateScale;

    bool bLoop;
    
public:
    UAnimDataModel* GetDataModel() const;
};
