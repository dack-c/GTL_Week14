#pragma once
#include "ParticleModuleTypeDataBase.h"
#include "VertexData.h"

class UStaticMesh;

// TypeData 중 Mesh
class UParticleModuleMesh : public UParticleModuleTypeDataBase
{
    DECLARE_CLASS(UParticleModuleMesh, UParticleModuleTypeDataBase)
public:
    UStaticMesh* Mesh = nullptr;
    bool bUseMeshMaterials = true;

    // 이 타입에 필요한 파티클 데이터 크기 반환
    virtual int32 GetRequiredParticleBytes() const
    {
        return sizeof(FVector)      // 위치
            + sizeof(FQuat)        // 회전
            + sizeof(FVector);     // 스케일
    }

    // 렌더링을 위한 정점 데이터 크기
    virtual int32 GetDynamicVertexStride() const
    {
        // 기본 Mesh의 정점 크기
        return sizeof(FNormalVertex);
    }

    void SetMesh(UStaticMesh* InMesh, class UParticleEmitter* OwnerEmitter)
    {
        Mesh = InMesh;

        if (!Mesh || !bUseMeshMaterials || OwnerEmitter == nullptr)
            return;

        // Required 모듈 찾아서 머티리얼 동기화
        if (auto* Required = OwnerEmitter->GetModule<UParticleModuleRequired>())
        {
            if (Mesh->HasMaterial())
            {
                FString MatName = Mesh->GetMeshGroupInfo()[0].PathFileName;
                UMaterial* DefaultMaterial = UResourceManager::GetInstance().Findmaterial(SlotName);
                if (DefaultMaterial)
                {
                    Required->Material = DefaultMaterial;
                }
            }
        }

        // 에미터 렌더 타입도 Mesh로 강제
        OwnerEmitter->RenderType = EParticleType::Mesh;
    }
};
