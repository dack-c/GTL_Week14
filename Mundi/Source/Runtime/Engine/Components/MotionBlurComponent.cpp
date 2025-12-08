#include "pch.h"
#include "MotionBlurComponent.h"
#include "BillboardComponent.h"
void UMotionBlurComponent::OnRegister(UWorld* InWorld)
{
	Super::OnRegister(InWorld);
	if (!SpriteComponent && !InWorld->bPie)
	{
		CREATE_EDITOR_COMPONENT(SpriteComponent, UBillboardComponent);
		SpriteComponent->SetTexture(GDataDir + "/UI/Icons/S_AtmosphericHeightFog.dds");
	}

}

void UMotionBlurComponent::RenderHeightFog(URenderer* Renderer)
{
}

void UMotionBlurComponent::Serialize(const bool bInIsLoading, JSON& InOutHandle)
{
	Super::Serialize(bInIsLoading, InOutHandle);
}

void UMotionBlurComponent::DuplicateSubObjects()
{
	Super::DuplicateSubObjects();
}
