#pragma once
#include "ParticleModule.h"

// Size By Life (라이프 기준 크기) 모듈
// 생애 비율 t에 따라 크기를 곱해주는 모듈
// Update에서 Size *= Curve(t)를 수행
class UParticleModuleSizeMultiplyLife : public UParticleModule
{
    DECLARE_CLASS(UParticleModuleSizeMultiplyLife, UParticleModule)
public:
    UParticleModuleSizeMultiplyLife();

    // Scale Curve(t) - t=0에서 0.2 → t=0.5에서 1.0 → t=1에서 0.0
    // 이런 식으로 "커졌다가 사라지는" 연출
    FRawDistributionVector LifeMultiplier = FRawDistributionVector(FVector(1.0f, 1.0f, 1.0f));

    // 축별(X/Y/Z)로 제어 가능 여부
    bool bMultiplyX = true;
    bool bMultiplyY = true;
    bool bMultiplyZ = true;

    // ============================================================
    // 언리얼 스타일 구현
    // ============================================================

    // Update에서 BaseSize에 Curve(t)를 곱해서 Size 애니메이션
    virtual void Update(FParticleEmitterInstance* Owner, int32 Offset, float DeltaTime) override;
};
