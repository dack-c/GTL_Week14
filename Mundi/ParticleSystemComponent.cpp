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
    ReleaseParticleBuffers();
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

void UParticleSystemComponent::ReleaseParticleBuffers()
{
    if (ParticleVertexBuffer)
    {
        ParticleVertexBuffer->Release();
        ParticleVertexBuffer = nullptr;
    }

    if (ParticleIndexBuffer)
    {
        ParticleIndexBuffer->Release();
        ParticleIndexBuffer = nullptr;
    }

    ParticleVertexCapacity = 0;
    ParticleIndexCount = 0;
}

void UParticleSystemComponent::CollectMeshBatches(TArray<FMeshBatchElement>& OutMeshBatchElements, const FSceneView* View)
{
    if (!IsVisible() || EmitterRenderData.IsEmpty())
    {
        return;
    }

    for (int32 EmitterIdx = 0; EmitterIdx < EmitterRenderData.Num(); ++EmitterIdx)
    {

        //FDynamicEmitterDataBase* Data = EmitterRenderData[EmitterIdx];
        // if (!Data || Data->ActiveParticleCount <= 0)        
        //{
        //    continue;
        //}
        
        // 일단은 Sprite만, TEST
        BuildParticleBatch(OutMeshBatchElements, View);

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

void UParticleSystemComponent::BuildParticleBatch(TArray<FMeshBatchElement>& OutMeshBatchElements, const FSceneView* View)
{
    const uint32 ParticleCount = static_cast<uint32>(EmitterRenderData.Num());
    if (ParticleCount == 0)
    {
        return;
    }

    const uint32 ClampedCount = MaxDebugParticles > 0
        ? std::min<uint32>(ParticleCount, static_cast<uint32>(MaxDebugParticles))
        : ParticleCount;

    if (!ParticleVertexBuffer || !ParticleIndexBuffer)
    {
        return;
    }

    D3D11RHI* RHIDevice = GEngine.GetRHIDevice();
    ID3D11DeviceContext* Context = RHIDevice ? RHIDevice->GetDeviceContext() : nullptr;
    if (!Context)
    {
        return;
    }

    D3D11_MAPPED_SUBRESOURCE Mapped = {};
    if (FAILED(Context->Map(ParticleVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Mapped)))
    {
        return;
    }

    FParticleSpriteVertex* Vertices = reinterpret_cast<FParticleSpriteVertex*>(Mapped.pData);
    static const FVector2D CornerOffsets[4] = {
        FVector2D(-1.0f, -1.0f),
        FVector2D(1.0f, -1.0f),
        FVector2D(1.0f,  1.0f),
        FVector2D(-1.0f,  1.0f)
    };

    uint32 VertexCursor = 0;
    for (uint32 ParticleIndex = 0; ParticleIndex < ClampedCount; ++ParticleIndex)
    {
        const FDynamicEmitterDataBase& Data = *EmitterRenderData[ParticleIndex];
        for (int CornerIndex = 0; CornerIndex < 4; ++CornerIndex)
        {
            FParticleSpriteVertex& Vertex = Vertices[VertexCursor++];
            Vertex.Position = Data.Position;
            Vertex.Corner = CornerOffsets[CornerIndex];
            Vertex.Size = Data.Size;
            Vertex.Color = Data.Color;
        }
    }

    Context->Unmap(ParticleVertexBuffer, 0);

    if (!ParticleMaterial)
    {
        ParticleMaterial = UResourceManager::GetInstance().Load<UMaterial>("Shaders/Particle/ParticleSprite.hlsl");
    }

    UShader* ParticleShader = nullptr;
    if (ParticleMaterial)
    {
        ParticleShader = ParticleMaterial->GetShader();
    }
    if (!ParticleShader)
    {
        ParticleShader = UResourceManager::GetInstance().Load<UShader>("Shaders/Particle/ParticleSprite.hlsl");
    }
    if (!ParticleShader)
    {
        return;
    }

    TArray<FShaderMacro> ShaderMacros = View ? View->ViewShaderMacros : TArray<FShaderMacro>();
    if (ParticleMaterial)
    {
        ShaderMacros.Append(ParticleMaterial->GetShaderMacros());
    }

    FShaderVariant* ShaderVariant = ParticleShader->GetOrCompileShaderVariant(ShaderMacros);
    if (!ShaderVariant)
    {
        return;
    }

    const int32 BatchIndex = OutMeshBatchElements.Add(FMeshBatchElement());
    FMeshBatchElement& Batch = OutMeshBatchElements[BatchIndex];
    Batch.VertexShader = ShaderVariant->VertexShader;
    Batch.PixelShader = ShaderVariant->PixelShader;
    Batch.InputLayout = ShaderVariant->InputLayout;
    Batch.Material = ParticleMaterial;
    Batch.VertexBuffer = ParticleVertexBuffer;
    Batch.IndexBuffer = ParticleIndexBuffer;
    Batch.VertexStride = sizeof(FParticleSpriteVertex);
    Batch.IndexCount = ClampedCount * 6;
    Batch.PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    Batch.WorldMatrix = GetWorldMatrix();
    Batch.ObjectID = InternalIndex;
}