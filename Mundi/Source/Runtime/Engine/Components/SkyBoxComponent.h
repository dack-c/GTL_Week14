#pragma once
#include "PrimitiveComponent.h"
#include "USkyBoxComponent.generated.h"


UCLASS(DisplayName = "스카이 박스", Description = "스카이 박스")
class USkyBoxComponent : public UPrimitiveComponent
{
	GENERATED_REFLECTION_BODY()

	static const FWideString SkyBoxPath;

public:

	USkyBoxComponent() = default;
	~USkyBoxComponent() = default;
protected:
	
};
