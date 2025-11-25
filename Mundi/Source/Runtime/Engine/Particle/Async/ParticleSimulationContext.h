#pragma once

struct FParticleSimulationContext
{
    // 시간 정보
    float DeltaTime;
    float RealTimeSeconds;

    // 공간 정보
    FVector ComponentLocation;
    FQuat   ComponentRotation;
    FVector ComponentScale;
    FMatrix ComponentWorldMatrix;

    // 카메라
    FVector CameraLocation;
    FQuat CameraRotation;

    // 상태 정보
    bool bIsActive;
    bool bSuppressSpawning;
    int32 CurrentLODIndex;
};
