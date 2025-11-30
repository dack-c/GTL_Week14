// DOF Tile Flatten Compute Shader
// 8x8 타일별 Min/Max CoC 계산

#define TILE_SIZE 8

Texture2D<float4> g_CoCTexture : register(t0);  // Setup 결과 (Far 또는 Near)
RWTexture2D<float4> g_TileOutput : register(u0);

cbuffer TileConstants : register(b0)
{
    uint2 InputSize;    // 입력 텍스처 크기 (1/4 해상도)
    uint2 TileCount;    // 타일 개수
};

groupshared float sharedMinCoC[TILE_SIZE * TILE_SIZE];
groupshared float sharedMaxCoC[TILE_SIZE * TILE_SIZE];

[numthreads(TILE_SIZE, TILE_SIZE, 1)]
void mainCS(
    uint3 GroupId : SV_GroupID,
    uint3 GroupThreadId : SV_GroupThreadID,
    uint GroupIndex : SV_GroupIndex)
{
    // 픽셀 좌표 계산
    uint2 pixelPos = GroupId.xy * TILE_SIZE + GroupThreadId.xy;

    // 경계 체크
    float coc = 0.0;
    if (all(pixelPos < InputSize))
    {
        coc = g_CoCTexture[pixelPos].a;  // Alpha = CoC
    }

    // Shared memory에 저장
    sharedMinCoC[GroupIndex] = coc;
    sharedMaxCoC[GroupIndex] = coc;

    GroupMemoryBarrierWithGroupSync();

    // Parallel reduction (64 -> 32 -> 16 -> 8 -> 4 -> 2 -> 1)
    [unroll]
    for (uint s = (TILE_SIZE * TILE_SIZE) / 2; s > 0; s >>= 1)
    {
        if (GroupIndex < s)
        {
            sharedMinCoC[GroupIndex] = min(sharedMinCoC[GroupIndex], sharedMinCoC[GroupIndex + s]);
            sharedMaxCoC[GroupIndex] = max(sharedMaxCoC[GroupIndex], sharedMaxCoC[GroupIndex + s]);
        }
        GroupMemoryBarrierWithGroupSync();
    }

    // Thread 0이 결과 저장
    if (GroupIndex == 0)
    {
        // x: MinCoC, y: MaxCoC, z: unused, w: unused
        g_TileOutput[GroupId.xy] = float4(sharedMinCoC[0], sharedMaxCoC[0], 0, 0);
    }
}
