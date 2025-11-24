#pragma once
#include "ParticleModule.h"

// Initial Rotation (초기 회전) 모듈
// 스폰 순간의 초기 회전각(보통 Z 회전)을 설정
// Spawn 단계에서 Particle.Rotation 초기값 생성
class UParticleModuleRotation : public UParticleModule
{
    DECLARE_CLASS(UParticleModuleRotation, UParticleModule)
public:
    UParticleModuleRotation();

    // 초기 회전각 (라디안 단위)
    // 랜덤 분포로 "각기 다른 방향으로 돌아간 파편" 느낌
    FRawDistributionFloat StartRotation = FRawDistributionFloat(0.0f);

    // ============================================================
    // 언리얼 스타일 구현
    // ============================================================

    // Spawn에서 초기 회전각 설정 - "처음부터 삐뚤게 놓고 시작하는" 역할
    virtual void Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase) override;
};
