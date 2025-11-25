#include "pch.h"
#include "ParticleModuleRibbon.h"

IMPLEMENT_CLASS(UParticleModuleRibbon)

UParticleModuleRibbon::UParticleModuleRibbon()
{
    ModuleType = EParticleModuleType::TypeData;
    TypeDataType = EParticleType::Ribbon;

    Width = 10.0f;
    TilingDistance = 0.0f;    // 0이면 전체를 0~1로 Stretch
    TrailLifetime = 1.0f;     // 기본 1초
    bUseCameraFacing = true;  // 기본은 카메라 향하게
}