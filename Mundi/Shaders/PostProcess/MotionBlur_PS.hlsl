Texture2D    g_SceneColorTex   : register(t0);
SamplerState g_LinearClampSample  : register(s0);

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

cbuffer PostProcessCB : register(b0)
{
    float Near;
    float Far;
}

cbuffer ViewProjBuffer : register(b1)
{
    row_major float4x4 ViewMatrix;
    row_major float4x4 ProjectionMatrix;
    row_major float4x4 InverseViewMatrix;
    row_major float4x4 InverseProjectionMatrix;
}

cbuffer MotionBlurCB : register(b2)
{
    float2 Center;          // 블러 중심점 (0.5, 0.5 = 화면 중앙)
    float  Intensity;       // 블러 강도 (0~1)
    int    SampleCount;     // 샘플 개수 (8~32)

    float  Weight;          // 전체 효과 가중치 (0~1)
    float3 _Pad0;
}

cbuffer ViewportConstants : register(b10)
{
    float4 ViewportRect;    // x: TopLeftX, y: TopLeftY, z: Width, w: Height
    float4 ScreenSize;      // x: Width, y: Height, z: 1/W, w: 1/H
}

float4 mainPS(PS_INPUT input) : SV_Target
{
    // 원본 색상
    float4 SceneColor = g_SceneColorTex.Sample(g_LinearClampSample, input.texCoord);

    // Weight가 0이면 원본 반환
    if (Weight <= 0.0f)
    {
        return SceneColor;
    }

    // Radial Blur: 중심점에서 현재 픽셀 방향으로 블러
    float2 Direction = input.texCoord - Center;
    float Distance = length(Direction);

    // 가장자리로 갈수록 블러 강하게 (제곱 함수 사용)
    float DistanceScale = saturate(Distance * 2.0f);
    DistanceScale = pow(DistanceScale, 1.5f);  // 가장자리 강조 (1.5 제곱)
    float FinalIntensity = Intensity * Weight * DistanceScale;

    // 샘플링 방향 벡터 (중심에서 바깥으로, 블러 거리 증가)
    float2 BlurVector = Direction * FinalIntensity * 0.06f;  // 0.02 → 0.06 (3배 증가)

    // 누적 색상
    float4 AccumColor = SceneColor;
    int ValidSamples = 1;

    // Radial Blur: 중심 방향으로 여러 샘플 취득
    for (int i = 1; i < SampleCount; ++i)
    {
        float t = (float)i / (float)SampleCount;
        float2 SampleUV = input.texCoord - BlurVector * t;

        // UV 범위 체크
        if (SampleUV.x >= 0.0f && SampleUV.x <= 1.0f &&
            SampleUV.y >= 0.0f && SampleUV.y <= 1.0f)
        {
            AccumColor += g_SceneColorTex.Sample(g_LinearClampSample, SampleUV);
            ValidSamples++;
        }
    }

    // 평균 색상 반환
    return AccumColor / (float)ValidSamples;
}
