#pragma once
#include "ParticleModuleTypeDataBase.h"

class UParticleModuleRibbon : public UParticleModuleTypeDataBase
{
	DECLARE_CLASS(UParticleModuleRibbon, UParticleModuleTypeDataBase)
public:
	UParticleModuleRibbon();
    void ApplyToEmitter(UParticleEmitter* OwnerEmitter);

    // 리본 전체 기본 폭 (World Space)
    float Width;

    // 리본 길이 방향 UV 타일링 거리 (월드 거리 -> U/V 1.0f)
    // 0 이면 "Stretch 전체 0~1" 방식
    float TilingDistance;

    // 트레일 전체 수명 (초 단위). 시뮬레이션에서 사용.
    float TrailLifetime;

    // 카메라 기준으로 리본 너비 방향을 잡을지 여부
    bool bUseCameraFacing;
};