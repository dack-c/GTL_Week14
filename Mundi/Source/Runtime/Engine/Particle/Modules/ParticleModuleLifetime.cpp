#include "pch.h"
#include "ParticleModuleLifetime.h"
#include "../ParticleEmitter.h"
#include "Source/Runtime/Engine/Particle/ParticleEmitterInstance.h"
#include "Source/Runtime/Engine/Particle/ParticleHelper.h"

IMPLEMENT_CLASS(UParticleModuleLifetime)

UParticleModuleLifetime::UParticleModuleLifetime()
{
    bSpawnModule = true;
}

void UParticleModuleLifetime::Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase)
{
    if (!ParticleBase)
        return;
    
    // 수명 설정
    float MaxLifetime = Lifetime.GetValue(Owner->GetRandomFloat());
    if (MaxLifetime > 0.0f)
    {
        ParticleBase->OneOverMaxLifetime = 1.0f / MaxLifetime;
    }
    else
    {
        ParticleBase->OneOverMaxLifetime = 0.0f;
    }

    ParticleBase->RelativeTime = 0.0f;
}

void UParticleModuleLifetime::Update(FParticleEmitterInstance* Owner, int32 Offset, float DeltaTime)
{
    // Lifetime은 보통 별도의 업데이트가 필요없음
    // RelativeTime은 다른 시스템에서 업데이트됨
    // 필요시 여기에 추가 로직 구현 가능
}
