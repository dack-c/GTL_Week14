#pragma once
// --------------------------------------------------------
// [통계 구조체] 파티클 시스템 현황
// --------------------------------------------------------
struct FParticleStats
{
    // 1. 파티클 총 개수
    uint32 TotalActiveParticles = 0;

    // 2. DrawCall (생성된 MeshBatch 수)
    uint32 DrawCalls = 0;


    void Reset()
    {
        TotalActiveParticles = 0;
        DrawCalls = 0;
    }
};

// --------------------------------------------------------
// [매니저] 전역 접근용 싱글톤
// --------------------------------------------------------
class FParticleStatManager
{
public:
    static FParticleStatManager& GetInstance()
    {
        static FParticleStatManager Instance;
        return Instance;
    }

    // 매 프레임(RenderStart 등)에서 호출하여 초기화
    void ResetStats() { CurrentStats.Reset(); }

    // 데이터 누적 (여러 컴포넌트가 있을 수 있으므로 +=)
    void AddParticleCount(uint32 Count) { CurrentStats.TotalActiveParticles += Count; }
    void AddDrawCalls(uint32 Count)     { CurrentStats.DrawCalls += Count; }

    const FParticleStats& GetStats() const { return CurrentStats; }

private:
    FParticleStatManager() = default;
    FParticleStats CurrentStats;
};
