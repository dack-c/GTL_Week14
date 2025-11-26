#include "pch.h"
#include "ParticleModuleBeam.h"
#include "../ParticleEmitter.h"

IMPLEMENT_CLASS(UParticleModuleBeam)

UParticleModuleBeam::UParticleModuleBeam()
{
    ModuleType = EParticleModuleType::TypeData;
    TypeDataType = EParticleType::Beam;
}

void UParticleModuleBeam::ApplyToEmitter(UParticleEmitter* OwnerEmitter)
{
    if (!OwnerEmitter)
    {
        return;
    }

    // Emitter의 RenderType을 Beam으로 설정
    OwnerEmitter->RenderType = EParticleType::Beam;
}
