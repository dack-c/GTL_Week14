#pragma once


enum class EParticleModuleType : uint8
{
    Required, Spawn, Update, TypeData
};

class UParticleModule : public UObject
{
public:
    EParticleModuleType ModuleType;
    bool  bEnabled = true;
    int32 SortPriority = 0;

    // payload가 필요한 모듈이면 바이트 수를 정의
    virtual int32 GetRequiredBytesPerParticle() const { return 0; }

    // Cache 단계에서 emitter가 채워줌(에셋 캐시)
    int32 PayloadOffset = -1;
};