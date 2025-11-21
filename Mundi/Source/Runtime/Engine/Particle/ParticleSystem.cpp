#include "pch.h"
#include "ParticleSystem.h"
#include "ParticleEmitter.h"

void UParticleSystem::BuildRuntimeCache()
{
    MaxActiveParticles = 0;
    MaxLifetime = 0.f;

    for (auto* Emitter : Emitters)
    {
        if (!Emitter) continue;
        Emitter->CacheEmitterModuleInfo();
        MaxActiveParticles += Emitter->MaxParticles;
        MaxLifetime = FMath::Max(MaxLifetime, Emitter->MaxLifetime);
    }
}
