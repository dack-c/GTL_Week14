#include "pch.h"
#include "ParticleModuleRotationRate.h"
#include "../ParticleEmitter.h"
#include "../ParticleHelper.h"

IMPLEMENT_CLASS(UParticleModuleRotationRate)

UParticleModuleRotationRate::UParticleModuleRotationRate()
{
    ModuleType = EParticleModuleType::Spawn;
    bSpawnModule = true;
    bUpdateModule = true;
}

void UParticleModuleRotationRate::Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase)
{
    if (!ParticleBase)
        return;

    // 랜덤 시드 생성 (파티클 카운터 기반)
    float RandomSeed = (float)(Owner->ParticleCounter % 1000) / 1000.0f;

    // 초기 회전 각도 설정 (라디안 단위)
    float InitialRotationValue = InitialRotation.GetValue(RandomSeed);
    ParticleBase->Rotation = InitialRotationValue;
    ParticleBase->BaseRotation = InitialRotationValue;

    // 초기 회전 속도 설정 (라디안/초 단위)
    float RotationRateValue = StartRotationRate.GetValue(RandomSeed);
    ParticleBase->RotationRate = RotationRateValue;
    ParticleBase->BaseRotationRate = RotationRateValue;
}

void UParticleModuleRotationRate::Update(FParticleEmitterInstance* Owner, int32 Offset, float DeltaTime)
{
    // BEGIN_UPDATE_LOOP 매크로 사용 - 모든 활성 파티클 순회
    BEGIN_UPDATE_LOOP
    {
        // Rotation += RotationRate * dt
        // 각속도를 시간에 적분하여 회전각 업데이트
        Particle.Rotation += Particle.RotationRate * DeltaTime;
    }
    END_UPDATE_LOOP;
}
