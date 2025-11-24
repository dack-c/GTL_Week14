#include "pch.h"
#include "ParticleModuleRotation.h"
#include "../ParticleEmitterInstance.h"
#include "../ParticleHelper.h"

IMPLEMENT_CLASS(UParticleModuleRotation)

UParticleModuleRotation::UParticleModuleRotation()
{
    ModuleType = EParticleModuleType::Spawn;
    bSpawnModule = true;
}

void UParticleModuleRotation::Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase)
{
    if (!ParticleBase)
        return;

    // 랜덤 시드 생성 (파티클 카운터 기반)
    float RandomSeed = (float)(Owner->ParticleCounter % 1000) / 1000.0f;

    // 초기 회전각 설정 (라디안 단위)
    float RotationValue = StartRotation.GetValue(RandomSeed);

    ParticleBase->Rotation = RotationValue;
    ParticleBase->BaseRotation = RotationValue;

    UE_LOG("ParticleModuleRotation::Spawn - Set rotation to: %f radians", RotationValue);
}
