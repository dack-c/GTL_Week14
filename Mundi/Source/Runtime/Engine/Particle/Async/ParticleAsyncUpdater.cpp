#include "pch.h"
#include "ParticleAsyncUpdater.h"

#include "PlatformTime.h"

FParticleAsyncUpdater::~FParticleAsyncUpdater()
{
    if (TaskHandle.valid()) TaskHandle.wait();
    InternalClearRenderData();
}

void FParticleAsyncUpdater::KickOff(const TArray<FParticleEmitterInstance*>& Instances, const FParticleSimulationContext& Context)
{
    if (IsBusy()) { return; }
    
    if (TaskHandle.valid())
    {
        FAsyncSimulationResult Result = TaskHandle.get();
        
        // 데이터 교체 (Swap)
        InternalClearRenderData();
        RenderData = Result.RenderData;
        LastFrameStats = Result.Stats;
    }
    
    TaskHandle = std::async(std::launch::async, [Instances, Context]() 
    {
        return DoSimulationWork(Instances, Context);
    });
}

void FParticleAsyncUpdater::KickOffSync(const TArray<FParticleEmitterInstance*>& Instances, const FParticleSimulationContext& Context)
{
    Sync();
    if (!RenderData.IsEmpty())
    {
        InternalClearRenderData();
    }
    FAsyncSimulationResult Result = DoSimulationWork(Instances, Context);
    // 새 데이터로 교체
    RenderData = Result.RenderData;

    // 통계 갱신
    LastFrameStats = Result.Stats;
}

void FParticleAsyncUpdater::EnsureCompletion()
{
    if (TaskHandle.valid())
    {
        TaskHandle.wait();
    }
}

void FParticleAsyncUpdater::Sync()
{
}

bool FParticleAsyncUpdater::TrySync()
{
    if (!TaskHandle.valid()) return false;

    // 즉시 상태 확인
    auto Status = TaskHandle.wait_for(std::chrono::seconds(0));
    if (Status == std::future_status::ready)
    {
        // 작업 완료
        FAsyncSimulationResult Result = TaskHandle.get();

        // 데이터 교체
        InternalClearRenderData();
        RenderData = Result.RenderData;
        LastFrameStats = Result.Stats;
            
        return true;
    }

    return false; // 아직 일하는 중 (기존 데이터 유지)
}

bool FParticleAsyncUpdater::IsBusy() const
{
    return TaskHandle.valid() && TaskHandle.wait_for(std::chrono::seconds(0)) != std::future_status::ready;
}

FAsyncSimulationResult FParticleAsyncUpdater::DoSimulationWork(const TArray<FParticleEmitterInstance*>& Instances, const FParticleSimulationContext& Context)
{
    TIME_PROFILE(Particle_Simulation)
    FAsyncSimulationResult Result;
        
    // 통계 초기화
    Result.Stats.bAllEmittersComplete = true;
    Result.Stats.TotalActiveParticles = 0;
    Result.Stats.bHasActiveParticles = false;

    const FVector ViewOrigin = Context.CameraLocation; // 혹은 Context.CameraLocation (별도 추가 권장)
    const FVector ViewDir = Context.CameraRotation.ToEulerZYXDeg(); // 혹은 Context.CameraForward

    for (int32 Idx = 0; Idx < Instances.Num(); ++Idx)
    {
        FParticleEmitterInstance* Inst = Instances[Idx];
        if (!Inst) continue;

        // 시뮬레이션 수행
        Inst->Tick(Context);

        // 통계 집계
        int32 Count = Inst->ActiveParticles;
        Result.Stats.TotalActiveParticles += Count;
            
        if (Count > 0) 
        {
            Result.Stats.bHasActiveParticles = true;
        }
            
        if (!Inst->IsComplete())
        {
            Result.Stats.bAllEmittersComplete = false;
        }

        // 렌더 데이터 생성
        FDynamicEmitterDataBase* EmitterData = Inst->CreateDynamicData();
        if (EmitterData)
        {
            EmitterData->EmitterIndex = Idx;
            if (EmitterData->EmitterType == EParticleType::Sprite)
            {
                auto* SpriteData = static_cast<FDynamicSpriteEmitterData*>(EmitterData);
                SpriteData->SortParticles(ViewOrigin, ViewDir, Context.ComponentWorldMatrix, SpriteData->AsyncSortedIndices);
            }
            else if (EmitterData->EmitterType == EParticleType::Mesh)
            {
                auto* MeshData = static_cast<FDynamicMeshEmitterData*>(EmitterData);
                MeshData->SortParticles(ViewOrigin, ViewDir, Context.ComponentWorldMatrix, MeshData->AsyncSortedIndices);
            }
            Result.RenderData.Add(EmitterData);
        }
    }

    return Result;
}

void FParticleAsyncUpdater::InternalClearRenderData()
{
    for (FDynamicEmitterDataBase* Data : RenderData)
    {
        if (Data)
        {
            delete Data;
        }
    }
    RenderData.Empty();
}
