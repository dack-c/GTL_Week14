#include "DOF_Common.hlsli"

// Input Textures
Texture2D    g_SceneColorTex : register(t0);  // Full Res
Texture2D    g_SceneDepthTex : register(t1);  // Full Res
Texture2D    g_FarFieldTex   : register(t2);  // 1/4 Res (Blurred)
Texture2D    g_NearFieldTex  : register(t3);  // 1/4 Res (Blurred)

SamplerState g_LinearClampSample : register(s0);
SamplerState g_PointClampSample  : register(s1);

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

// Constant Buffers
cbuffer PostProcessCB : register(b0)
{
    float Near;
    float Far;
    int IsOrthographic;
    float Padding0;
}

cbuffer ViewProjBuffer : register(b1)
{
    row_major float4x4 ViewMatrix;
    row_major float4x4 ProjectionMatrix;
    row_major float4x4 InverseViewMatrix;
    row_major float4x4 InverseProjectionMatrix;
}

cbuffer DOFRecombineCB : register(b2)
{
    float FocalDistance;           // m (meters)
    float Fstop;
    float SensorWidth;             // m (e.g., 0.024 = 24mm)
    float FocalRegion;             // m

    float NearTransitionRegion;    // m
    float FarTransitionRegion;     // m
    float MaxNearBlurSize;         // pixels
    float MaxFarBlurSize;          // pixels

    float NearClip;
    float FarClip;
    int IsOrthographic2;
    float _Pad0;

    float2 ViewRectMinUV;          // ViewRect 시작 UV (게임 영역)
    float2 ViewRectMaxUV;          // ViewRect 끝 UV (게임 영역)
}

cbuffer ViewportConstants : register(b10)
{
    float4 ViewportRect;
    float4 ScreenSize;
}

// 출력
struct PS_OUTPUT
{
    float4 Color : SV_Target0;
};

// Bilateral Upsampling: 깊이 기반 가중치로 엣지 보존 업샘플링
float4 BilateralUpsample(Texture2D blurTex, float2 uv, float centerDepth, float2 lowResTexelSize)
{
    float4 blur = 0;
    float totalWeight = 0;

    [unroll]
    for (int y = -1; y <= 1; y++)
    {
        [unroll]
        for (int x = -1; x <= 1; x++)
        {
            float2 sampleUV = uv + float2(x, y) * lowResTexelSize;
            float4 s = blurTex.Sample(g_LinearClampSample, sampleUV);

            float sampleDepth = g_SceneDepthTex.Sample(g_PointClampSample, sampleUV).r;
            sampleDepth = LinearizeDepth(sampleDepth, NearClip, FarClip, IsOrthographic2);

            float depthDiff = abs(sampleDepth - centerDepth) / max(centerDepth, 0.1);
            float w = exp(-depthDiff * 5.0);
            blur += s * w;
            totalWeight += w;
        }
    }

    return blur / max(totalWeight, 0.001);
}

PS_OUTPUT mainPS(PS_INPUT input)
{
    PS_OUTPUT output;

    // 1. 원본 씬 컬러 샘플링
    float4 sceneColor = g_SceneColorTex.Sample(g_PointClampSample, input.texCoord);

    // 2. ViewRect 영역 체크 (게임 영역만 DOF 적용, UI 영역은 원본)
    bool inViewRect = all(input.texCoord >= ViewRectMinUV) && all(input.texCoord <= ViewRectMaxUV);
    if (!inViewRect)
    {
        output.Color = sceneColor;
        return output;
    }

    // 3. Depth 샘플링 및 CoC 계산
    float rawDepth = g_SceneDepthTex.Sample(g_PointClampSample, input.texCoord).r;
    float viewDepth = LinearizeDepth(rawDepth, NearClip, FarClip, IsOrthographic2);

    float CoC = CalculateCoC(
        viewDepth,
        FocalDistance,
        Fstop,
        SensorWidth,
        FocalRegion,
        NearTransitionRegion,
        FarTransitionRegion,
        MaxNearBlurSize,
        MaxFarBlurSize
    );

    // 4. CoC가 0이면 원본 그대로 (선명 영역)
    if (abs(CoC) < 0.001)
    {
        output.Color = sceneColor;
        return output;
    }

    // 5. Bilateral Upsampling으로 블러 텍스처 샘플링 (엣지 보존)
    float2 lowResTexelSize = ScreenSize.zw * 4.0;  // 1/4 해상도 텍셀 크기
    float4 farField = BilateralUpsample(g_FarFieldTex, input.texCoord, viewDepth, lowResTexelSize);
    float4 nearField = BilateralUpsample(g_NearFieldTex, input.texCoord, viewDepth, lowResTexelSize);

    // 6. Smoothstep 블렌딩 (경계 부드럽게)
    float blendFactor = saturate(abs(CoC));
    blendFactor = blendFactor * blendFactor * (3.0 - 2.0 * blendFactor);  // Smoothstep

    float4 finalColor;

    if (CoC > 0.0)
    {
        // 원경 (Far Field)
        finalColor = lerp(sceneColor, float4(farField.rgb, 1.0), blendFactor);
    }
    else  // CoC < 0.0
    {
        // 근경 (Near Field)
        finalColor = lerp(sceneColor, float4(nearField.rgb, 1.0), blendFactor);
    }

    output.Color = finalColor;

    return output;
}
