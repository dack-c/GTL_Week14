#pragma once
#include "PrimitiveComponent.h"
#include "UParticleSystemComponent.generated.h"

// Render 확인하기 위한 Driver!!
// 지우셔도 됩니다!!
UCLASS(DisplayName = "파티클 컴포넌트", Description = "드라이버")
class UParticleSystemComponent : public UPrimitiveComponent
{
public: 
	GENERATED_REFLECTION_BODY();

};