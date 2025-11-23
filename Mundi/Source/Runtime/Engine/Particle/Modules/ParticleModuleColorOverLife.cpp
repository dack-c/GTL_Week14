#include "pch.h"
#include "ParticleModuleColorOverLife.h"
#include "../ParticleEmitter.h"
#include "../ParticleHelper.h"

IMPLEMENT_CLASS(UParticleModuleColorOverLife)

UParticleModuleColorOverLife::UParticleModuleColorOverLife()
{
    ModuleType = EParticleModuleType::Update;
    bUpdateModule = true;
}

void UParticleModuleColorOverLife::Update(FParticleEmitterInstance* Owner, int32 Offset, float DeltaTime)
{
    // BEGIN_UPDATE_LOOP 매크로 사용 - 모든 활성 파티클 순회
    BEGIN_UPDATE_LOOP
    {
        // RelativeTime은 0~1 값 (Age / Lifetime)
        float t = Particle.RelativeTime;

        // Color(t) 커브에서 값을 가져와 Particle.Color에 적용
        if (bUseColorOverLife)
        {
            FLinearColor NewColor = ColorOverLife.GetValue(t);
            Particle.Color.R = NewColor.R;
            Particle.Color.G = NewColor.G;
            Particle.Color.B = NewColor.B;
        }

        // Alpha(t) 커브에서 값을 가져와 Particle.Color.A에 적용
        if (bUseAlphaOverLife)
        {
            Particle.Color.A = AlphaOverLife.GetValue(t);
        }
    }
    END_UPDATE_LOOP;
}
