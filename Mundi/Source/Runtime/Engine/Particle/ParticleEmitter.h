#pragma once


class UParticleLODLevel;


class UParticleEmitter : public UObject
{
public:
    void CacheEmitterModuleInfo();
    
public:
    TArray<UParticleLODLevel*> LODLevels;

    int32 ParticleSizeBytes = 0;
    int32 MaxParticles = 0;
    float MaxLifetime = 0.f;
    
};