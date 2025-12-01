// DOF Scatter Vertex Shader
// Near Field 픽셀을 CoC 크기의 Point Sprite로 확장

struct VS_INPUT
{
    uint VertexID : SV_VertexID;      // 0~3 (쿼드 코너)
    uint InstanceID : SV_InstanceID;  // 픽셀 인덱스
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;   // Bokeh 내 UV (0~1)
    float3 Color    : TEXCOORD1;
    float  CocRadius: TEXCOORD2;
};

// Near Field 텍스처
Texture2D<float4> g_NearFieldTex : register(t0);

cbuffer ScatterCB : register(b0)
{
    float2 TexelSize;      // 1/Width, 1/Height (Half Res)
    float2 ScreenSize;     // Width, Height (Half Res)
    float  MaxBlurRadius;  // 최대 블러 반경 (pixels)
    float  CocThreshold;   // Scatter 임계값 (0.1 = CoC 10% 이상만)
    float  BokehIntensity; // 보케 밝기 배율
    uint   PixelOffset;    // 배치 시작 픽셀 인덱스
};

// 쿼드 정점 오프셋 - Triangle Strip CW 순서
// 0: 좌하, 1: 좌상, 2: 우하, 3: 우상
// Triangle 0: 0,1,2 (CW), Triangle 1: 2,1,3 (CW)
static const float2 QuadOffsets[4] = {
    float2(-1, -1),  // 0: 좌하
    float2(-1,  1),  // 1: 좌상
    float2( 1, -1),  // 2: 우하
    float2( 1,  1)   // 3: 우상
};

static const float2 QuadUVs[4] = {
    float2(0, 1),  // 0: 좌하
    float2(0, 0),  // 1: 좌상
    float2(1, 1),  // 2: 우하
    float2(1, 0)   // 3: 우상
};

// 디버그 모드: 1=강제 쿼드 그리기, 0=원래 로직
#define DEBUG_FORCE_SCATTER 0

VS_OUTPUT mainVS(VS_INPUT input)
{
    VS_OUTPUT output;

    // InstanceID = 배치 내 픽셀 인덱스, VertexID = 쿼드 코너 (0~3)
    // SV_InstanceID는 항상 0부터 시작하므로 PixelOffset으로 보정
    uint pixelIndex = input.InstanceID + PixelOffset;
    uint cornerIndex = input.VertexID;

    uint pixelX = pixelIndex % (uint)ScreenSize.x;
    uint pixelY = pixelIndex / (uint)ScreenSize.x;

    // 텍스처에서 색상과 CoC 읽기
    float4 texData = g_NearFieldTex.Load(int3(pixelX, pixelY, 0));
    float3 color = texData.rgb;
    float coc = texData.a;  // 0~1 정규화

#if DEBUG_FORCE_SCATTER 
    // ===== 디버그: 무조건 쿼드 그리기 =====
    // 매 200번째 픽셀만 그려서 성능 문제 방지
    if (pixelIndex % 200 != 0)
    {
        output.Position = float4(10.0, 10.0, 0.0, 1.0);
        output.TexCoord = float2(0, 0);
        output.Color = float3(0, 0, 0);
        output.CocRadius = 0;
        return output;
    }

    float2 centerUV = (float2(pixelX, pixelY) + 0.5) * TexelSize;
    float2 centerNDC = centerUV * 2.0 - 1.0;
    centerNDC.y = -centerNDC.y;

    // 실제 CoC 기반 크기 (최소 4픽셀로 보이게)
    float debugRadius = max(coc * MaxBlurRadius, 4.0);
    float2 radiusNDC = debugRadius * TexelSize * 2.0;
    float2 vertexNDC = centerNDC + QuadOffsets[cornerIndex] * radiusNDC;

    output.Position = float4(vertexNDC, 0.5, 1.0);
    output.TexCoord = QuadUVs[cornerIndex];

    // CoC 값에 따라 색상 변경
    // 빨강 = CoC 있음 (Near blur 영역)
    // 파랑 = CoC 없음
    // 초록 = 텍스처 읽기 자체 실패 체크용
    if (coc > 0.01)
        output.Color = float3(coc * 2.0, 0, 0);  // 빨간색 (CoC 강도)
    else if (color.r > 0.01 || color.g > 0.01 || color.b > 0.01)
        output.Color = float3(0, 1, 0);  // 초록색 (색상은 있지만 CoC 없음)
    else
        output.Color = float3(0, 0, 1);  // 파란색 (데이터 없음)

    output.CocRadius = debugRadius;
    return output;
    // =====================================
#else
    // 원래 로직
    // Scatter는 큰 CoC만 (Gather로 커버 못하는 엣지 확장용)
    float cocRadiusPx = coc * MaxBlurRadius;
    bool shouldScatter = (coc > CocThreshold) && (cocRadiusPx > 3.0);

    if (!shouldScatter)
    {
        output.Position = float4(10.0, 10.0, 0.0, 1.0);
        output.TexCoord = float2(0, 0);
        output.Color = float3(0, 0, 0);
        output.CocRadius = 0;
        return output;
    }

    float2 centerUV = (float2(pixelX, pixelY) + 0.5) * TexelSize;
    float2 centerNDC = centerUV * 2.0 - 1.0;
    centerNDC.y = -centerNDC.y;

    float2 radiusNDC = cocRadiusPx * TexelSize * 2.0;
    float2 vertexNDC = centerNDC + QuadOffsets[cornerIndex] * radiusNDC;

    output.Position = float4(vertexNDC, 0.5, 1.0);
    output.TexCoord = QuadUVs[cornerIndex];
    output.Color = color * BokehIntensity;
    output.CocRadius = cocRadiusPx;
#endif

    return output;
}
