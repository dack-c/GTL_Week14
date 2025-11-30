// DOF Tile Dilate Compute Shader
// 이웃 타일로 Min/Max CoC 확장

#define TILE_SIZE 8
#define DILATE_RADIUS 2  // 2 = 최대 16픽셀(2타일) 거리까지 확인

Texture2D<float4> g_TileInput : register(t0);   // Flatten 결과
RWTexture2D<float4> g_TileOutput : register(u0);

cbuffer TileConstants : register(b0)
{
    uint2 TileCount;    // 타일 개수
    float CocRadiusToTileScale;  // CoC를 타일 거리로 변환하는 스케일
    float _Pad0;
};

[numthreads(8, 8, 1)]
void mainCS(uint3 DTid : SV_DispatchThreadID)
{
    if (any(DTid.xy >= TileCount))
        return;

    // 현재 타일의 Min/Max
    float4 centerTile = g_TileInput[DTid.xy];
    float minCoC = centerTile.x;
    float maxCoC = centerTile.y;

    // Ring별로 이웃 타일 확인
    // Ring 1: 거리 1 타일 (8픽셀)
    // Ring 2: 거리 2 타일 (16픽셀)
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
                if (any(neighborPos < 0) || any(neighborPos >= int2(TileCount)))
                    continue;

                float4 neighborTile = g_TileInput[neighborPos];
                float neighborMin = neighborTile.x;
                float neighborMax = neighborTile.y;

                // 이웃의 CoC가 이 거리까지 도달할 수 있는지 체크
                // CoC 반경 >= 타일 경계까지 거리 이면 영향 있음
                if (neighborMax * CocRadiusToTileScale > ringDistance)
                {
                    maxCoC = max(maxCoC, neighborMax);
                }

                if (abs(neighborMin) * CocRadiusToTileScale > ringDistance)
                {
                    minCoC = min(minCoC, neighborMin);
                }
            }
        }
    }

    // 결과 저장
    // x: Dilated MinCoC, y: Dilated MaxCoC
    g_TileOutput[DTid.xy] = float4(minCoC, maxCoC, 0, 0);
}
