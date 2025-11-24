#include "pch.h"
#include "ParticleModuleLifetime.h"
#include "../ParticleEmitter.h"
#include "Source/Runtime/Engine/Particle/ParticleEmitterInstance.h"
#include "Source/Runtime/Engine/Particle/ParticleHelper.h"

IMPLEMENT_CLASS(UParticleModuleLifetime)

UParticleModuleLifetime::UParticleModuleLifetime()
{
    bSpawnModule = true;
    bUpdateModule = false; // Lifetime은 ParticleEmitter에서 자동으로 처리됨
    ModuleType = EParticleModuleType::Spawn;
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
        // 수명이 0 이하면 즉시 죽는 파티클 (무한 수명)
        ParticleBase->OneOverMaxLifetime = 0.0f;
    }

    // 초기 상대 시간 (0.0 = 방금 생성됨, 1.0 = 수명 다함)
    ParticleBase->RelativeTime = 0.0f;
}

void UParticleModuleLifetime::Update(FParticleEmitterInstance* Owner, int32 Offset, float DeltaTime)
{
    // Lifetime 모듈은 Update 단계가 필요 없음
    //
    // RelativeTime 업데이트와 파티클 제거는 ParticleEmitter::Tick()의
    // 메인 루프에서 자동으로 처리됨:
    //   - Particle->RelativeTime += Particle->OneOverMaxLifetime * DeltaTime;
    //   - if (Particle->RelativeTime >= 1.0f) KillParticle(i);
    //
    // 따라서 이 함수는 비어있어도 정상 동작함
}
