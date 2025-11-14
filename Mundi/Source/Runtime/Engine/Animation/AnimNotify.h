#pragma once
#include "Object.h"

class USkeletalMeshComponent;
class UAnimSequenceBase;

class UAnimNotify : public UObject
{
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation);  

};
