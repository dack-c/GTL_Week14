#pragma once
#include "AnimNotify.h"
#include "Source/Runtime/Engine/Audio/Sound.h"

class UAnimNotify_PlaySound : public UAnimNotify
{
public:
	USound* Sound;
	 
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

};
