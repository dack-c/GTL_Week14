#pragma once

#include "SceneComponent.h"
#include "UTriggerComponent.generated.h"

UCLASS(DisplayName = "TriggerComponent", Description = "트리거 컴포넌트")
class UTriggerComponent : public USceneComponent
{
public:

	GENERATED_REFLECTION_BODY();
	UTriggerComponent();
	~UTriggerComponent() override;
	void Serialize(const bool bInIsLoading, JSON& InOutHandle) override;
	virtual void DuplicateSubObjects() override;
	void TickComponent(float DeltaTime) override;       // 매 프레임
	void RenderDebugVolume(class URenderer* Renderer) const override;
	void OnRegister(UWorld* InWorld) override;

	UPROPERTY(EditAnywhere, Category = "Trigger")
	FString LuaFunctionName;
	UPROPERTY(EditAnywhere, Category = "BoxExtent")
	FVector BoxExtent; // Half Extent

	FVector4 DebugVolumeColor = FVector4(0.2f, 0.8f, 1.0f, 1.0f);
	static FVector CharacterPos; //하드코딩 렛츠고

private:
	bool bWasInside = false;  // 이전 프레임에 트리거 내부에 있었는지

public:

};