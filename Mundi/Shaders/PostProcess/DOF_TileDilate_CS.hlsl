// DOF Tile Dilate Compute Shader
// 이웃 타일로 Far/Near Min/Max CoC 확장

#define TILE_SIZE 8
#define DILATE_RADIUS 4  // 2 = 최대 16픽셀(2타일) 거리까지 확인

Texture2D<float4> g_TileInput : register(t0);   // Flatten 결과 (x: FarMax, y: NearMax, z: FarMin, w: NearMin)
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

[numthreads(8, 8, 1)]
void mainCS(uint3 DTid : SV_DispatchThreadID)
{
    if (DTid.x >= TileCountX || DTid.y >= TileCountY)
        return;

    // 현재 타일의 Far/Near Min/Max CoC
    float4 centerTile = g_TileInput[DTid.xy];
    float farMaxCoC = centerTile.x;
    float nearMaxCoC = centerTile.y;
    float farMinCoC = centerTile.z;
    float nearMinCoC = centerTile.w;

    // Ring별로 이웃 타일 확인
    [unroll]
    for (int ring = 1; ring <= DILATE_RADIUS; ring++)
    {
        float ringDistance = float(ring - 1) * TILE_SIZE;  // 타일 경계까지 거리 (픽셀)

        // 정사각형 링 패턴으로 순회
        for (int y = -ring; y <= ring; y++)
        {
            for (int x = -ring; x <= ring; x++)
            {
                // 링 외곽만 (내부는 이미 처리됨)
                if (abs(x) != ring && abs(y) != ring)
                    continue;

                int2 neighborPos = int2(DTid.xy) + int2(x, y);

                // 경계 체크
                if (neighborPos.x < 0 || neighborPos.y < 0 ||
                    neighborPos.x >= (int)TileCountX || neighborPos.y >= (int)TileCountY)
                    continue;

                float4 neighborTile = g_TileInput[neighborPos];
                float neighborFarMax = neighborTile.x;
                float neighborNearMax = neighborTile.y;
                float neighborFarMin = neighborTile.z;
                float neighborNearMin = neighborTile.w;

                // 이웃의 CoC가 이 거리까지 도달할 수 있는지 체크
                if (neighborFarMax * CocRadiusToTileScale > ringDistance)
                {
                    farMaxCoC = max(farMaxCoC, neighborFarMax);
                    // Min은 유효한 값만 (0보다 큰 경우)
                    if (neighborFarMin > 0.001)
                    {
                        farMinCoC = (farMinCoC > 0.001) ? min(farMinCoC, neighborFarMin) : neighborFarMin;
                    }
                }

                if (neighborNearMax * CocRadiusToTileScale > ringDistance)
                {
                    nearMaxCoC = max(nearMaxCoC, neighborNearMax);
                    if (neighborNearMin > 0.001)
                    {
                        nearMinCoC = (nearMinCoC > 0.001) ? min(nearMinCoC, neighborNearMin) : neighborNearMin;
                    }
                }
            }
        }
    }

    // 결과 저장
    // x: Dilated FarMaxCoC, y: Dilated NearMaxCoC, z: Dilated FarMinCoC, w: Dilated NearMinCoC
    g_TileOutput[DTid.xy] = float4(farMaxCoC, nearMaxCoC, farMinCoC, nearMinCoC);
}
