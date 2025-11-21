#pragma once
#include "ParticleModule.h"


class UParticleModuleRequired : public UParticleModule
{
public:
    // ---- Emitter 운영 ----
    int32 MaxParticles = 1000;

    float EmitterDuration = 1.0f;
    float EmitterDelay    = 0.0f;
    int32 EmitterLoops    = 0;     // 0 = infinite (너희 규칙대로)

    // (MVP 편의) 기본 스폰레이트 fallback
    float SpawnRateBase = 10.0f;

    // ---- 공간/종료 규칙 ----
    bool bUseLocalSpace    = false;
    bool bKillOnDeactivate = true;
    bool bKillOnCompleted  = false;

    // ---- 렌더 기본 ----
    UMaterial* Material = nullptr;
    EBlendMode BlendMode = EBlendMode::Alpha;

    EScreenAlignment ScreenAlignment = EScreenAlignment::CameraFacing;
    ESortMode SortMode = ESortMode::ByDistance;

    // ---- (선택) 기본 초기값 fallback ----
    FRawDistributionVector InitialSize;
    FRawDistributionColor  InitialColor;
    FRawDistributionFloat  InitialRotation;
};
