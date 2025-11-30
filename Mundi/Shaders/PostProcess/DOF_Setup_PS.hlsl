#include "DOF_Common.hlsli"

// Input Textures
Texture2D    g_SceneColorTex : register(t0);
Texture2D    g_SceneDepthTex : register(t1);

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

cbuffer DOFSetupCB : register(b2)
{
    float FocalDistance;           // m (meters)
    float FocalRegion;             // m
    float NearTransitionRegion;    // m
    float FarTransitionRegion;     // m

    float MaxNearBlurSize;         // pixels
    float MaxFarBlurSize;          // pixels
    float NearClip;
    float FarClip;

    int IsOrthographic2;
    float3 _Pad0;
}

cbuffer ViewportConstants : register(b10)
{
    float4 ViewportRect;
    float4 ScreenSize;
}

// MRT 출력 (Far Field, Near Field)
struct PS_OUTPUT
{
    float4 FarField  : SV_Target0;
    float4 NearField : SV_Target1;
};

PS_OUTPUT mainPS(PS_INPUT input)
{
    PS_OUTPUT output;

    // 1. Bilateral 다운샘플 (깊이 기반 가중치로 엣지 보존)
    float2 fullResTexelSize = ScreenSize.zw;  // 1.0 / ScreenSize

    // depthSigma: 깊이 감도 (m 단위, 작을수록 엣지 보존 강함)
    // 일반적으로 0.5~2.0m 정도가 적절
    float depthSigma = 1.0;

    BilateralDownsampleResult dsResult = BilateralDownsampleWithCoC(
        g_SceneColorTex,
        g_SceneDepthTex,
        g_LinearClampSample,
        g_PointClampSample,
        input.texCoord,
        fullResTexelSize,
        NearClip,
        FarClip,
        IsOrthographic2,
        depthSigma,
        FocalDistance,
        FocalRegion,
        NearTransitionRegion,
        FarTransitionRegion,
        MaxNearBlurSize,
        MaxFarBlurSize
    );

    float4 sceneColor = dsResult.color;
    float CoC = dsResult.coc;
    float linearDepth = dsResult.linearDepth;

    // 헤일로 방지: Sky Focus Distance (부드러운 전환)
    // 매우 먼 거리(하늘/빈 공간)는 블러 점진적 감소
    const float skyFocusStart = FarClip * 0.5;   // 전환 시작 (50%)
    const float skyFocusEnd = FarClip * 0.9;     // 전환 끝 (90%)

    if (linearDepth > skyFocusStart && CoC > 0.0)
    {
        // smoothstep으로 부드럽게 CoC 감소
        float skyFalloff = 1.0 - smoothstep(skyFocusStart, skyFocusEnd, linearDepth);
        CoC *= skyFalloff;
    }

    // 2. Near/Far 분리 (CoC 기준으로 한쪽에만 출력 - 색 섞임 방지)
    if (CoC > 0.0)
    {
        // 원경: Far에만
        output.FarField = float4(sceneColor.rgb, CoC);
        output.NearField = float4(0, 0, 0, 0);
    }
    else if (CoC < 0.0)
    {
        // 근경: Near에만
        output.FarField = float4(0, 0, 0, 0);
        output.NearField = float4(sceneColor.rgb, -CoC);
    }
    else
    {
        // 초점: 둘 다 X
        output.FarField = float4(0, 0, 0, 0);
        output.NearField = float4(0, 0, 0, 0);
    }

    return output;
}
