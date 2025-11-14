#pragma once
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

    FString SkinningType = "test";

    FSkinningStats() {};

    FSkinningStats(const FSkinningStats& other)
    {
        TotalSkeletals = other.TotalSkeletals;
        TotalBones = other.TotalBones;
        TotalVertices = other.TotalVertices;
        SelectedBones = other.SelectedBones;
        SelectedVertices = other.SelectedVertices;
        SkinningType = other.SkinningType;        
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
        TotalBones = 0;
        TotalVertices = 0;
        SelectedBones = 0;
        SelectedVertices = 0;
        SkinningType.clear();
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

   

private:
    FSkinningStatManager() = default;
    ~FSkinningStatManager() = default;
    FSkinningStatManager(const FSkinningStatManager&) = delete;
    FSkinningStatManager& operator=(const FSkinningStatManager&) = delete;

    FSkinningStats CurrentStats;
};