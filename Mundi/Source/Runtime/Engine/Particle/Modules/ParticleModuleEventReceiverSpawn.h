#pragma once
#include "ParticleModule.h"

class UParticleModuleEventReceiverSpawn : public UParticleModule
{
public:
    DECLARE_CLASS(UParticleModuleEventReceiverSpawn, UParticleModule)

public:
    UParticleModuleEventReceiverSpawn();
    
    void UpdateAsync(FParticleEmitterInstance* Owner, int32 Offset, FParticleSimulationContext& Context) override;
    
// Module Attribute
public:
    // 수신할 이벤트의 이름
    FName EventName; 

    // 하나의 이벤트 발생 시 스폰할 파티클의 수
    int32 Count = 1;

    // 스폰할 파티클 수의 랜덤 범위 (Count ~ Count + CountRange)
    int32 CountRange = 0;

    float InitialSpeed = 10.0f;

    // 스폰된 파티클의 속도에 이벤트 방향을 반영할지 여부
    bool bUseEmitterDirection = false; 

    // 스폰될 파티클의 위치에 이벤트 위치를 사용할지 여부
    bool bUseEmitterLocation = true;
};
