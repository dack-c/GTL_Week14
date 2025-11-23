#pragma once
#include "ResourceBase.h"

class UParticleEmitter;
class UParticleLODLevel;
class UParticleModule;

/**
 * @brief 파티클 시스템을 정의하는 에셋 단위의 클래스
 * Resource로 관리되어 ResourceManager에 등록되어 관리
 * 하위의 UParticleEmitter는 UParticleSystem이 관리
 * 그 하위의 UParticleModule은 UParticleLODLevel이 관리
 */
class UParticleSystem : public UResourceBase
{
    DECLARE_CLASS(UParticleSystem, UObject)
public:
    UParticleSystem();
    ~UParticleSystem() override;
    
    void BuildRuntimeCache();

    TArray<UParticleEmitter*> Emitters;
    int32 MaxActiveParticles = 0;
    float MaxLifetime = 0.f;

// Add Section
    UParticleEmitter* AddEmitter(UClass* EmitterClass);
    
// JSON Serialization Section
public:
    // UResourceBase Load
    void Load(const FString& InFilePath, ID3D11Device* InDevice);
    
    // ParticleSystem을 JSON 형식으로 파일에 저장
    bool SaveToFile(const FString& FilePath);

    void Serialize(const bool bInIsLoading, JSON& InOutHandle) override;
};