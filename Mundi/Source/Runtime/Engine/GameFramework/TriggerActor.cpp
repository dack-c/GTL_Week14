#include "pch.h"
#include "TriggerActor.h"
#include "TriggerComponent.h"
#include "LuaScriptComponent.h"

ATriggerActor::ATriggerActor()
{
	ObjectName = "TriggerActor";
	TriggerComponent = CreateDefaultSubobject<UTriggerComponent>("Trigger");
	SetRootComponent(TriggerComponent);
	UActorComponent* ActorComp = AddNewComponent(UClass::FindClass("ULuaScriptComponent"), nullptr);
	
}

ATriggerActor::~ATriggerActor()
{
}

void ATriggerActor::DuplicateSubObjects()
{
	Super::DuplicateSubObjects();
}

void ATriggerActor::Serialize(const bool bInIsLoading, JSON& InOutHandle)
{
	Super::Serialize(bInIsLoading, InOutHandle);
}
