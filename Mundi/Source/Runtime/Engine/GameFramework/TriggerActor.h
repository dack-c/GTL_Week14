#pragma once

#include "Actor.h"
#include "ATriggerActor.generated.h"

class UTriggerComponent;
class ULuaScriptComponent;

UCLASS(DisplayName = "트리거 액터", Description = "트리거 액터")
class ATriggerActor : public AActor
{
public:
	GENERATED_REFLECTION_BODY()
	ATriggerActor();
protected:
	~ATriggerActor() override;

public:
	void DuplicateSubObjects() override;

	void Serialize(const bool bInIsLoading, JSON& InOutHandle) override;

protected:
	UTriggerComponent* TriggerComponent;
	ULuaScriptComponent* LuaComponent;
};
