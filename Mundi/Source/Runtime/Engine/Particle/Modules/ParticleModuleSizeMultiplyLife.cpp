#include "pch.h"
#include "ParticleModuleSizeMultiplyLife.h"
#include "../ParticleEmitter.h"
#include "../ParticleHelper.h"

IMPLEMENT_CLASS(UParticleModuleSizeMultiplyLife)

UParticleModuleSizeMultiplyLife::UParticleModuleSizeMultiplyLife()
{
    ModuleType = EParticleModuleType::Update;
    bUpdateModule = true;
    bSpawnModule = false;
}

FVector UParticleModuleSizeMultiplyLife::EvaluateSizeCurve(float t) const
{
    // t는 파티클의 절대 시간 (Age)

    // 점1 이전: Point1Value로 고정
    if (t < Point1Time)
    {
        return Point1Value;
    }
    // 점2 이후: Point2Value로 고정
    else if (t >= Point2Time)
    {
        return Point2Value;
    }
    // 점1과 점2 사이: 선형 보간
    else
    {
        float alpha = (t - Point1Time) / (Point2Time - Point1Time);
        return FMath::Lerp(Point1Value, Point2Value, alpha);
    }
}

void UParticleModuleSizeMultiplyLife::Update(FParticleEmitterInstance* Owner, int32 Offset, float DeltaTime)
{
    // BEGIN_UPDATE_LOOP 매크로 사용 - 모든 활성 파티클 순회
    BEGIN_UPDATE_LOOP
    {
        // RelativeTime (0~1)에서 절대 시간 계산
        // Lifetime = 1.0 / OneOverMaxLifetime
        float Lifetime = 1.0f / Particle.OneOverMaxLifetime;
        float t = Particle.RelativeTime * Lifetime;

        // 2개 키포인트 커브에서 크기 배율 계산
        FVector SizeMultiplier = EvaluateSizeCurve(t);

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
