#include "pch.h"
#include "ParticleModuleVelocity.h"
#include "../ParticleEmitter.h"
#include "../ParticleHelper.h"
#include "Source/Runtime/Engine/Particle/ParticleEmitterInstance.h"
#include "Source/Runtime/Engine/Components/ParticleSystemComponent.h"

IMPLEMENT_CLASS(UParticleModuleVelocity)

UParticleModuleVelocity::UParticleModuleVelocity()
{
    bSpawnModule = true;
    bUpdateModule = true;
}

void UParticleModuleVelocity::Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase)
{
    if (!ParticleBase)
        return;

    // 초기 속도 설정
    FVector LocalVelocity = StartVelocity.GetValue({Owner->GetRandomFloat(), Owner->GetRandomFloat(),Owner->GetRandomFloat()});
    float Multiplier = VelocityMultiplier.GetValue(Owner->GetRandomFloat());

    // 컴포넌트의 월드 회전을 적용하여 월드 공간 속도 벡터 계산
    FQuat ComponentRot = Owner->Component->GetWorldRotation();
    FVector WorldVelocity = ComponentRot.RotateVector(LocalVelocity);

    ParticleBase->Velocity = WorldVelocity * Multiplier;
    ParticleBase->BaseVelocity = ParticleBase->Velocity;
}

void UParticleModuleVelocity::Update(FParticleEmitterInstance* Owner, int32 Offset, float DeltaTime)
{
    BEGIN_UPDATE_LOOP
    {
        // 이전 위치 저장
        Particle.OldLocation = Particle.Location;

        // 중력 적용
        Particle.Velocity += Gravity * DeltaTime;

        // 공기 저항 적용
        if (Damping > 0.0f)
        {
            float DampingFactor = FMath::Max(0.0f, 1.0f - Damping * DeltaTime);
            Particle.Velocity *= DampingFactor;
        }

        // 위치 업데이트
        Particle.Location += Particle.Velocity * DeltaTime;
    }
    END_UPDATE_LOOP;
}
