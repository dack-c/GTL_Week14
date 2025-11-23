#pragma once
#include "ParticleModule.h"

// Color Over Life (컬러 오버 라이프) 모듈
// 나이(t=Age/Lifetime)에 따라 색과 알파가 변하는 모듈
// Update 단계에서 매 프레임 색을 재계산
class UParticleModuleColorOverLife : public UParticleModule
{
    DECLARE_CLASS(UParticleModuleColorOverLife, UParticleModule)
public:
    UParticleModuleColorOverLife();

    // Color(t) 커브 - 시작은 흰색 → 중간 노랑 → 끝에 빨강
    FRawDistributionColor ColorOverLife = FRawDistributionColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));

    // Alpha(t) 커브 - 페이드 인/아웃 구현의 핵심
    FRawDistributionFloat AlphaOverLife = FRawDistributionFloat(1.0f);

    // 기능 활성화 플래그
    bool bUseColorOverLife = true;
    bool bUseAlphaOverLife = true;

    // ============================================================
    // 언리얼 스타일 구현
    // ============================================================

    // Update에서 RelativeTime(0~1)에 따라 Color와 Alpha 재계산
    virtual void Update(FParticleEmitterInstance* Owner, int32 Offset, float DeltaTime) override;
};
