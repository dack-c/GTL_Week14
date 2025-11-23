#pragma once

class UParticleModule;
class UParticleModuleRequired;
class UParticleModuleSpawn;
class UParticleModuleTypeDataBase;

class UParticleLODLevel : public UObject
{
    DECLARE_CLASS(UParticleLODLevel, UObject)

public:
    UParticleLODLevel();
    ~UParticleLODLevel() override;

    void Serialize(const bool bInIsLoading, JSON& InOutHandle) override;

    UParticleModule* AddModule(UClass* ParticleModuleClass);
    void RebuildModuleCaches();
    
public:
    UParticleModuleRequired* RequiredModule = nullptr;
    UParticleModuleSpawn* SpawnModule = nullptr;
    UParticleModuleTypeDataBase* TypeDataModule = nullptr;

    TArray<UParticleModule*> SpawnModules;
    TArray<UParticleModule*> UpdateModules;

    // 전체 접근용 캐시
    TArray<UParticleModule*> AllModulesCache;

    int32 LODIndex = 0;
    bool  bEnabled = true;

private:
    void ParseAndAddModule(JSON& ModuleJson);
    JSON SerializeModule(UParticleModule* Module);
};