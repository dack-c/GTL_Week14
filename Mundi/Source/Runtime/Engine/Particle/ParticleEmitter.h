#pragma once

class UParticleLODLevel;
class UParticleSystemComponent;

class UParticleEmitter : public UObject
{
    DECLARE_CLASS(UParticleEmitter, UObject)

public:
    UParticleEmitter();
    ~UParticleEmitter() override;

    UParticleLODLevel* AddLODLevel(int32 LODIndex = 0);
    void CacheEmitterModuleInfo();
    void Serialize(const bool bInIsLoading, JSON& InOutHandle) override;
    template<typename T>
    T* GetModule(int32 LODIndex = 0) const
    {
        // LOD 존재 검증
        if (LODLevels.Num() <= LODIndex || LODLevels[LODIndex] == nullptr)
            return nullptr;

        const UParticleLODLevel* LOD = LODLevels[LODIndex];

        // 캐시가 비었다면 Rebuild 필요
        if (LOD->AllModulesCache.Num() == 0)
            return nullptr;

        for (UParticleModule* M : LOD->AllModulesCache)
        {
            if (!M || !M->bEnabled)
                continue;

            // 템플릿 타입과 정확히 매칭되는 모듈 가져오기
            if (auto* Casted = dynamic_cast<T*>(M))
                return Casted;
        }

        return nullptr;
    }
    
public:
    TArray<UParticleLODLevel*> LODLevels;
    EParticleType RenderType = EParticleType::Sprite;

    // Mesh 타입인 경우에만 사용
    UStaticMesh* Mesh = nullptr;
    bool bUseMeshMaterials = true; // Mesh의 머티리얼 그대로 쓸지 여부
    // 나중에 필요하면 Material도 여기 두거나 RequiredModule에서 가져오기~

    int32 ParticleSizeBytes = 0;
    int32 MaxParticles = 0;
    float MaxLifetime = 0.f;
};
