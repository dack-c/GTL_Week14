#include "pch.h"
#include "ParticleModuleSizeMultiplyLife.h"
#include "../ParticleEmitter.h"
#include "../ParticleHelper.h"

IMPLEMENT_CLASS(UParticleModuleSizeMultiplyLife)

UParticleModuleSizeMultiplyLife::UParticleModuleSizeMultiplyLife()
{
    ModuleType = EParticleModuleType::Update;
    bUpdateModule = true;
}

void UParticleModuleSizeMultiplyLife::Update(FParticleEmitterInstance* Owner, int32 Offset, float DeltaTime)
{
    // BEGIN_UPDATE_LOOP 매크로 사용 - 모든 활성 파티클 순회
    BEGIN_UPDATE_LOOP
    {
        // RelativeTime은 0~1 값 (Age / Lifetime)
        float t = Particle.RelativeTime;

        // Scale Curve(t)에서 값을 가져와 BaseSize에 곱함
        FVector SizeMultiplier = LifeMultiplier.GetValue(t);

        // 축별로 제어 가능
        if (bMultiplyX)
            Particle.Size.X = Particle.BaseSize.X * SizeMultiplier.X;
        else
            Particle.Size.X = Particle.BaseSize.X;

        if (bMultiplyY)
            Particle.Size.Y = Particle.BaseSize.Y * SizeMultiplier.Y;
        else
            Particle.Size.Y = Particle.BaseSize.Y;

        if (bMultiplyZ)
            Particle.Size.Z = Particle.BaseSize.Z * SizeMultiplier.Z;
        else
            Particle.Size.Z = Particle.BaseSize.Z;
    }
    END_UPDATE_LOOP;
}
