// DOF Tile Flatten Compute Shader
// 8x8 타일별 Min/Max CoC 계산 (Far + Near)

#define TILE_SIZE 8

Texture2D<float4> g_FarFieldTex  : register(t0);  // Far Field (CoC in alpha)
Texture2D<float4> g_NearFieldTex : register(t1);  // Near Field (CoC in alpha)
RWTexture2D<float4> g_TileOutput : register(u0);

cbuffer TileConstants : register(b0)
{
    uint InputSizeX;
    uint InputSizeY;
    uint TileCountX;
    uint TileCountY;
    float CocRadiusToTileScale;
    float3 _Pad;
};

// Far/Near 각각 Min/Max CoC 저장
groupshared float sharedFarMaxCoC[TILE_SIZE * TILE_SIZE];
groupshared float sharedNearMaxCoC[TILE_SIZE * TILE_SIZE];
groupshared float sharedFarMinCoC[TILE_SIZE * TILE_SIZE];
groupshared float sharedNearMinCoC[TILE_SIZE * TILE_SIZE];

[numthreads(TILE_SIZE, TILE_SIZE, 1)]
void mainCS(
    uint3 GroupId : SV_GroupID,
    uint3 GroupThreadId : SV_GroupThreadID,
    uint GroupIndex : SV_GroupIndex)
{
    // 픽셀 좌표 계산
    uint2 pixelPos = GroupId.xy * TILE_SIZE + GroupThreadId.xy;

    // 경계 체크 및 Far/Near CoC 읽기
    float farCoC = 0.0;
    float nearCoC = 0.0;
    if (pixelPos.x < InputSizeX && pixelPos.y < InputSizeY)
    {
        farCoC = g_FarFieldTex[pixelPos].a;   // Far Field CoC
        nearCoC = g_NearFieldTex[pixelPos].a; // Near Field CoC (양수화됨)
    }

    // Shared memory에 저장
    // Min은 CoC > 0인 픽셀만 고려 (0이면 큰 값으로 초기화)
    sharedFarMaxCoC[GroupIndex] = farCoC;
    sharedNearMaxCoC[GroupIndex] = nearCoC;
    sharedFarMinCoC[GroupIndex] = (farCoC > 0.001) ? farCoC : 999999.0;
    sharedNearMinCoC[GroupIndex] = (nearCoC > 0.001) ? nearCoC : 999999.0;

    GroupMemoryBarrierWithGroupSync();

    // Parallel reduction (64 -> 32 -> 16 -> 8 -> 4 -> 2 -> 1)
    [unroll]
    for (uint s = (TILE_SIZE * TILE_SIZE) / 2; s > 0; s >>= 1)
    {
        if (GroupIndex < s)
        {
            sharedFarMaxCoC[GroupIndex] = max(sharedFarMaxCoC[GroupIndex], sharedFarMaxCoC[GroupIndex + s]);
            sharedNearMaxCoC[GroupIndex] = max(sharedNearMaxCoC[GroupIndex], sharedNearMaxCoC[GroupIndex + s]);
            sharedFarMinCoC[GroupIndex] = min(sharedFarMinCoC[GroupIndex], sharedFarMinCoC[GroupIndex + s]);
            sharedNearMinCoC[GroupIndex] = min(sharedNearMinCoC[GroupIndex], sharedNearMinCoC[GroupIndex + s]);
        }
        GroupMemoryBarrierWithGroupSync();
    }

    // Thread 0이 결과 저장
    if (GroupIndex == 0)
    {
        // Min이 999999면 유효한 CoC가 없었음 → 0으로 저장
        float farMin = (sharedFarMinCoC[0] < 999999.0) ? sharedFarMinCoC[0] : 0.0;
        float nearMin = (sharedNearMinCoC[0] < 999999.0) ? sharedNearMinCoC[0] : 0.0;

        // x: FarMaxCoC, y: NearMaxCoC, z: FarMinCoC, w: NearMinCoC
        g_TileOutput[GroupId.xy] = float4(sharedFarMaxCoC[0], sharedNearMaxCoC[0], farMin, nearMin);
    }
}
