#include "pch.h"
#include "SkySphereActor.h"
#include "SkySphereComponent.h"
#include "ObjectFactory.h"

ASkySphereActor::ASkySphereActor()
{
	SkySphereComponent = CreateDefaultSubobject<USkySphereComponent>(FName("SkySphereComponent"));
	SetRootComponent(SkySphereComponent);

	bCanEverTick = true;

	PrevZenithColor = ZenithColor;
	PrevHorizonColor = HorizonColor;
	PrevGroundColor = GroundColor;
	PrevSunColor = SunColor;
	PrevSunDiskSize = SunDiskSize;
	PrevHorizonFalloff = HorizonFalloff;
	PrevOverallBrightness = OverallBrightness;
}

void ASkySphereActor::BeginPlay()
{
	Super::BeginPlay();
	SyncParametersToComponent();
}

void ASkySphereActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasParametersChanged())
	{
		SyncParametersToComponent();

		PrevZenithColor = ZenithColor;
		PrevHorizonColor = HorizonColor;
		PrevGroundColor = GroundColor;
		PrevSunColor = SunColor;
		PrevSunDiskSize = SunDiskSize;
		PrevHorizonFalloff = HorizonFalloff;
		PrevOverallBrightness = OverallBrightness;
	}
}

void ASkySphereActor::Serialize(const bool bInIsLoading, JSON& InOutHandle)
{
	Super::Serialize(bInIsLoading, InOutHandle);

	if (bInIsLoading)
	{
		SkySphereComponent = Cast<USkySphereComponent>(RootComponent);
	}
}

void ASkySphereActor::DuplicateSubObjects()
{
	Super::DuplicateSubObjects();

	for (UActorComponent* Component : OwnedComponents)
	{
		if (USkySphereComponent* Sky = Cast<USkySphereComponent>(Component))
		{
			SkySphereComponent = Sky;
		}
	}
}

void ASkySphereActor::SyncParametersToComponent()
{
	if (!SkySphereComponent)
		return;

	SkySphereComponent->ZenithColor = ZenithColor;
	SkySphereComponent->HorizonColor = HorizonColor;
	SkySphereComponent->GroundColor = GroundColor;
	SkySphereComponent->SunColor = SunColor;
	SkySphereComponent->SunDiskSize = SunDiskSize;
	SkySphereComponent->HorizonFalloff = HorizonFalloff;
	SkySphereComponent->OverallBrightness = OverallBrightness;
	SkySphereComponent->bAutoUpdateSunDirection = bAutoUpdateSunDirection;

	if (!bAutoUpdateSunDirection)
	{
		SkySphereComponent->SunDirection = ManualSunDirection.GetNormalized();
	}
}

bool ASkySphereActor::HasParametersChanged() const
{
	if (ZenithColor != PrevZenithColor) return true;
	if (HorizonColor != PrevHorizonColor) return true;
	if (GroundColor != PrevGroundColor) return true;
	if (SunColor != PrevSunColor) return true;
	if (SunDiskSize != PrevSunDiskSize) return true;
	if (HorizonFalloff != PrevHorizonFalloff) return true;
	if (OverallBrightness != PrevOverallBrightness) return true;
	return false;
}
