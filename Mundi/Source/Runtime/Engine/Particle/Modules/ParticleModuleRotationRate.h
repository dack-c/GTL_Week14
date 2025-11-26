#pragma once
#include "ParticleModule.h"

// Initial Rotation Rate (초기 회전 속도) 모듈
// 스폰 순간 회전 속도(각속도)를 설정
// Spawn 단계에서 RotationRate 잡고, Update에서 각도에 적분
class UParticleModuleRotationRate : public UParticleModule
{
    DECLARE_CLASS(UParticleModuleRotationRate, UParticleModule)
public:
    UParticleModuleRotationRate();

    // 초기 회전 속도 (라디안/초 단위)
    // 랜덤 각속도로 자연스러운 회전 버라이어티
    FRawDistributionVector StartRotationRate = FRawDistributionVector(FVector(0.0f, 0.0f, 0.0f));

    // 초기 회전 각도 (라디안 단위)
    // 파티클이 스폰될 때의 초기 각도 설정 (회전 속도와 함께 사용 가능)
    FRawDistributionVector InitialRotation = FRawDistributionVector(FVector(0.0f, 0.0f, 0.0f));

    // ============================================================
    // 언리얼 스타일 구현
    // ============================================================

    // Spawn에서 초기 회전 속도 설정
    virtual void Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase) override;

    // Update에서 Rotation += RotationRate * dt
    virtual void Update(FParticleEmitterInstance* Owner, int32 Offset, float DeltaTime) override;
};
