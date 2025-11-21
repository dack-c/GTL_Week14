#pragma once

class UParticleEmitter;

class UParticleSystem : public UObject
{
public:
    void BuildRuntimeCache();
    
public:
    TArray<UParticleEmitter*> Emitters;
    int32 MaxActiveParticles = 0;
    float MaxLifetime = 0.f;
};