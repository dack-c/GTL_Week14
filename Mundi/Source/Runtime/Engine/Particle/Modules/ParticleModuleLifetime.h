#pragma once
#include "ParticleModule.h"

// Lifetime (수명) 모듈
// 파티클이 생성된 후 얼마나 오래 살아있을지 결정
// 스폰 시 랜덤 수명을 설정하고, RelativeTime (0~1)을 계산하는 데 사용됨
class UParticleModuleLifetime : public UParticleModule
{
    DECLARE_CLASS(UParticleModuleLifetime, UParticleModule)
public:
    UParticleModuleLifetime();

    // 파티클 수명 (초 단위)
    // Min~Max 범위 사용 시 각 파티클마다 랜덤 수명 부여
    // 예: Lifetime(1.0f, 5.0f) → 1초~5초 사이 랜덤 수명
    FRawDistributionFloat Lifetime = FRawDistributionFloat(1.0f, 5.0f);

    // ============================================================
    // 언리얼 스타일 구현
    // ============================================================

    // Spawn: 파티클 생성 시 수명 설정 (OneOverMaxLifetime, RelativeTime 초기화)
    virtual void Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase) override;

    // Update: 사용 안 함 (RelativeTime은 ParticleEmitter에서 자동 업데이트)
    virtual void Update(FParticleEmitterInstance* Owner, int32 Offset, float DeltaTime) override;
};
