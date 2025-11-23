#pragma once
#include "ParticleModule.h"

// Size By Life (라이프 기준 크기) 모듈
// 파티클 수명에 따라 크기를 변화시키는 모듈
// 2개의 키포인트로 크기 애니메이션 제어
// 점1 이전: 수평선, 점1-점2: 보간, 점2 이후: 수평선
class UParticleModuleSizeMultiplyLife : public UParticleModule
{
    DECLARE_CLASS(UParticleModuleSizeMultiplyLife, UParticleModule)
public:
    UParticleModuleSizeMultiplyLife();

    // 첫 번째 키포인트 (시간, 값)
    float Point1Time = 0.0f;   // 시간 (0.0 ~ 100.0)
    FVector Point1Value = FVector(0.0f, 0.0f, 0.0f);  // 크기 배율

    // 두 번째 키포인트 (시간, 값)
    float Point2Time = 1.0f;   // 시간 (0.0 ~ 100.0)
    FVector Point2Value = FVector(100.0f, 100.0f, 100.0f);  // 크기 배율

    // 축별(X/Y/Z)로 제어 가능 여부
    bool bMultiplyX = true;
    bool bMultiplyY = true;
    bool bMultiplyZ = true;

    // ============================================================
    // 언리얼 스타일 구현
    // ============================================================

    // Update에서 BaseSize에 Curve(t)를 곱해서 Size 애니메이션
    virtual void Update(FParticleEmitterInstance* Owner, int32 Offset, float DeltaTime) override;

private:
    // t (0~1)에 따라 3개 키프레임 사이를 선형 보간
    FVector EvaluateSizeCurve(float t) const;
};
