#include "pch.h"
#include "ParticleModuleEventReceiverSpawn.h"
#include "Source/Runtime/Engine/Particle/ParticleEmitterInstance.h"

IMPLEMENT_CLASS(UParticleModuleEventReceiverSpawn)

UParticleModuleEventReceiverSpawn::UParticleModuleEventReceiverSpawn()
{
    bSpawnModule = false;
    bUpdateModule = true;
}

void UParticleModuleEventReceiverSpawn::UpdateAsync(FParticleEmitterInstance* Owner, int32 Offset, FParticleSimulationContext& Context)
{
    for (auto& Event: Context.EventData)
    {
        if (!EventName.IsValid() || Event.EventName == EventName)
        {
            int32 SpawnCount = Count;
            
            if (CountRange > 0)
            {
                float RandomFactor = Owner->GetRandomFloat(); 
                SpawnCount += static_cast<int32>(RandomFactor * CountRange);
            }

            if (SpawnCount > 0)
            {
                FVector InitialLocation = Event.HitResult.ImpactPoint; // 이벤트 발생 위치
                FVector InitialVelocity = FVector::Zero();

                if (bUseEmitterDirection)
                {
                    InitialVelocity = Event.HitResult.ImpactNormal * InitialSpeed; 
                }
                    
                Owner->SpawnParticles(SpawnCount, 0.0f, 0.0f,
                    InitialLocation, InitialVelocity, Context);
            }
        }
    }
}
