#include "pch.h"
#include "ParticleSystemComponent.h"
#include "Source/Runtime/Engine/Particle/DynamicEmitterDataBase.h"
#include "MeshBatchElement.h"
#include "SceneView.h"

UParticleSystemComponent::UParticleSystemComponent()
{
    // FOR TEST !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    bAutoActivate = true;

    // 디버그용 파티클 3개
    FDynamicEmitterDataBase P0;
    P0.Position = FVector(0, 0, 100);
    P0.Size = 30.0f;
    P0.Color = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
    EmitterRenderData.Add(&P0);

    FDynamicEmitterDataBase P1;
    P1.Position = FVector(100, 0, 100);
    P1.Size = 40.0f;
    P1.Color = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);
    EmitterRenderData.Add(&P1);

    FDynamicEmitterDataBase P2;
    P2.Position = FVector(0, 100, 150);
    P2.Size = 50.0f;
    P2.Color = FLinearColor(0.0f, 0.0f, 1.0f, 1.0f);
    EmitterRenderData.Add(&P2);

    MaxDebugParticles = 128; 
}

UParticleSystemComponent::~UParticleSystemComponent()
{
    DestroyParticles();
}

void UParticleSystemComponent::InitParticles()
{
    DestroyParticles();
    if (!Template) { return; }

    // 에셋에 정의된 이미터 개수만큼 인스턴스 생성
    for (int32 i = 0; i < Template->Emitters.Num(); i++)
    {
        UParticleEmitter* EmitterAsset = Template->Emitters[i];
        
        // 이미터가 유효하고 켜져있다면
        if (EmitterAsset)
        {
            FParticleEmitterInstance* NewInst = new FParticleEmitterInstance();
            NewInst->Init(EmitterAsset, this);
            EmitterInstances.Add(NewInst);
        }
    }

    if (bAutoActivate) { ActivateSystem(); }
}

void UParticleSystemComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);

    if (!bIsActive || !Template) return;

    bool bAllEmittersComplete = true;
    for (FParticleEmitterInstance* Inst : EmitterInstances)
    {
        Inst->Tick(DeltaTime);

        if (!Inst->IsComplete())
        {
            bAllEmittersComplete = false;
        }
    }

    if (bAllEmittersComplete)
    {
        OnParticleSystemFinished.Broadcast(this);
        DeactivateSystem();
        
        if (bAutoDestroy)
        {
            GetOwner()->Destroy();
        }
    }
}

void UParticleSystemComponent::DestroyParticles()
{
    for (FParticleEmitterInstance* Inst : EmitterInstances)
    {
        if (Inst)
        {
            Inst->FreeParticleMemory();
            delete Inst;
        }
    }
    EmitterInstances.Empty();
    bIsActive = false;
}

void UParticleSystemComponent::CollectMeshBatches(TArray<FMeshBatchElement>& OutMeshBatchElements, const FSceneView* View)
{
    if (!IsVisible() || EmitterRenderData.IsEmpty())
    {
        return;
    }

    for (int32 EmitterIdx = 0; EmitterIdx < EmitterRenderData.Num(); ++EmitterIdx)
    {

        FDynamicEmitterDataBase* Data = EmitterRenderData[EmitterIdx];
        // if (!Data || Data->ActiveParticleCount <= 0)
        if (!Data)
        {
            continue;
        }
        
        // 일단은 Sprite만, TEST
        BuildParticleBatch(*Data, OutMeshBatchElements, View);

        //switch (Data->EmitterType)
        //{
        //case EDynamicEmitterType::Sprite:
        //    BuildSpriteParticleBatch(static_cast<FDynamicSpriteEmitterDataBase&>(*Data),
        //        OutMeshBatchElements, View);
        //    break;
        //case EDynamicEmitterType::Mesh:
        //    BuildMeshParticleBatch(static_cast<FDynamicMeshEmitterDataBase&>(*Data),
        //        OutMeshBatchElements, View);
        //    break;
        //}
    }
}

void UParticleSystemComponent::BuildParticleBatch(const FDynamicEmitterDataBase& SpriteData,
    TArray<FMeshBatchElement>& OutMeshBatchElements, const FSceneView* View)
{
    // 임시로 Quad로 만들어뒀음
    // 파티클마다 위치 / 크기 / 색이 달라서, 동적 VB로 데이터를 뿌리고 코너를 다르게 적용해야함 -> InputLayout에 Corner 있는 이유
    static UQuad* ParticleQuad = nullptr;
    if (!ParticleQuad)
    {
        ParticleQuad = UResourceManager::GetInstance().Get<UQuad>("BillboardQuad");
        if (!ParticleQuad)
        {
            ParticleQuad = UResourceManager::GetInstance().Load<UQuad>("BillboardQuad");
        }
    }
    if (!ParticleQuad || ParticleQuad->GetIndexCount() == 0)
    {
        return;
    }

    UMaterialInterface* Material = GetMaterial(0);
    if (!Material)
    {
        Material = UResourceManager::GetInstance().GetDefaultMaterial();
    }
    if (!Material || !Material->GetShader())
    {
        return;
    }

    TArray<FShaderMacro> ShaderMacros = View ? View->ViewShaderMacros : TArray<FShaderMacro>();
    ShaderMacros.Append(Material->GetShaderMacros());
    FShaderVariant* ShaderVariant = Material->GetShader()->GetOrCompileShaderVariant(ShaderMacros);
    if (!ShaderVariant)
    {
        return;
    }

    FMeshBatchElement BatchElement;

    // 정렬 키
    BatchElement.VertexShader = ShaderVariant->VertexShader;
    BatchElement.PixelShader = ShaderVariant->PixelShader;
    BatchElement.InputLayout = ShaderVariant->InputLayout;
    BatchElement.Material = Material;
    BatchElement.VertexBuffer = ParticleQuad->GetVertexBuffer();
    BatchElement.IndexBuffer = ParticleQuad->GetIndexBuffer();
    BatchElement.VertexStride = ParticleQuad->GetVertexStride();

    BatchElement.IndexCount = ParticleQuad->GetIndexCount();
    BatchElement.PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    const FMatrix ParticleTransform = FMatrix::MakeScale(FVector(SpriteData.Size)) * FMatrix::MakeTranslation(SpriteData.Position);
    BatchElement.WorldMatrix = ParticleTransform * GetWorldMatrix();

    BatchElement.InstanceColor = SpriteData.Color;
    BatchElement.ObjectID = InternalIndex;

    OutMeshBatchElements.Add(BatchElement);
}