#pragma once
#include "SelectionManager.h"
#include "SkinnedMeshComponent.h"
#include "StatsOverlayD2D.h"
#include "UEContainer.h"
/*
 * 전체 bone 개수
 * 전체 vertex 개수
 * 전체 시간
 * 선택된 bone 개수
 * 선택된 vertex 개수
 * 선택된 놈 시간
 */
struct FSkinningStats
{
    // 전체 스켈레탈 개수
    uint32 TotalSkeletals = 0;
    // 전제 bone 개수
    uint32 TotalBones = 0;
    // 전체 정점 개수
    uint32 TotalVertices = 0;

    // Viewer의 Bone 개수(Selected Skeletal Bones)
    uint32 SelectedBones = 0;
    uint32 SelectedVertices = 0;

    FString AllSkeletalSkinningType = "CPU";
    FString SelectedSkeletalSkinningType = "NONE";

    FSkinningStats() {};

    FSkinningStats(const FSkinningStats& other)
    {
        TotalSkeletals = other.TotalSkeletals;
        TotalBones = other.TotalBones;
        TotalVertices = other.TotalVertices;
        SelectedBones = other.SelectedBones;
        SelectedVertices = other.SelectedVertices;
        AllSkeletalSkinningType = other.AllSkeletalSkinningType;
        SelectedSkeletalSkinningType = other.SelectedSkeletalSkinningType;
    };

    void AddStats(const FSkinningStats& other)
    {
        TotalSkeletals += other.TotalSkeletals;
        TotalBones += other.TotalBones;
        TotalVertices += other.TotalVertices;
        SelectedBones += other.SelectedBones;
        SelectedVertices += other.SelectedVertices;         
    }

    void Reset()
    {
        TotalSkeletals = 0;
        TotalBones = 0;
        TotalVertices = 0;
        SelectedBones = 0;
        SelectedVertices = 0;
        // AllSkeletalSkinningType = "CPU";
        // SelectedSkeletalSkinningType = "CPU";
    }
};

class FSkinningStatManager
{
public:
    static FSkinningStatManager& GetInstance()
    {
        static FSkinningStatManager Instance;
        return Instance;
    }

    void UpdateStats(const FSkinningStats& InStats)
    {
        CurrentStats = InStats;
    }

    const FSkinningStats& GetStats() const
    {
        return CurrentStats;
    }

    void ResetStats()
    {
        CurrentStats.Reset();
    }

    void SetSelectedSkinningType(const FString& InType)
    {
        CurrentStats.SelectedSkeletalSkinningType = InType;
    }

    void SetAllSkinningType(const FString& InType)
    {
        CurrentStats.AllSkeletalSkinningType = InType;
    }

    void GatherSkinnningStats(TArray<UMeshComponent*>& Components, USelectionManager* SelectionManager)
    {
        if(Components.IsEmpty() || !UStatsOverlayD2D::Get().IsSkinningVisible() || !SelectionManager)
        {
            return;
        }

        for (UMeshComponent* MeshComp : Components)
        {
            if (USkinnedMeshComponent* SkinnedComp = Cast<USkinnedMeshComponent>(MeshComp))
            {
                CurrentStats.TotalSkeletals++;
                if (USkeletalMesh* SkeletalMesh = SkinnedComp->GetSkeletalMesh())
                {
                    CurrentStats.TotalBones += SkeletalMesh->GetBoneCount();
                    CurrentStats.TotalVertices += SkeletalMesh->GetVertexCount();
                }
            }
        }

        if (USkinnedMeshComponent* SkinnedComp = Cast<USkinnedMeshComponent>(SelectionManager->GetSelectedComponent()))
        {
            if (USkeletalMesh* SkeletalMesh = SkinnedComp->GetSkeletalMesh())
            {
                CurrentStats.SelectedBones += SkeletalMesh->GetBoneCount();
                CurrentStats.SelectedVertices += SkeletalMesh->GetVertexCount();				
            }

            CurrentStats.SelectedSkeletalSkinningType = SkinnedComp->IsGPUSkinningEnable() ? "GPU" : "CPU"; 

        }
        else
        {
            CurrentStats.SelectedBones = 0;
            CurrentStats.SelectedVertices = 0;
            CurrentStats.SelectedSkeletalSkinningType = "NONE";
        }
    }

private:
    FSkinningStatManager() = default;
    ~FSkinningStatManager() = default;
    FSkinningStatManager(const FSkinningStatManager&) = delete;
    FSkinningStatManager& operator=(const FSkinningStatManager&) = delete;

    FSkinningStats CurrentStats;
};