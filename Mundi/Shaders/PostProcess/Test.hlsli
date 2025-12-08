// Input Textures (Near 또는 Far 분리 텍스처)
Texture2D g_SceneColor : register(t0);
Texture2D g_VelocityTex : register(t1);

SamplerState g_LinearClampSample : register(s0);
SamplerState g_PointClampSample : register(s1);

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

cbuffer ViewProjBuffer : register(b1)
{
    row_major float4x4 ViewMatrix;
    row_major float4x4 ProjectionMatrix;
    row_major float4x4 InverseViewMatrix;
    row_major float4x4 InverseProjectionMatrix;
};

cbuffer MotionBlurLastVPCB : register(b2)
{
    row_major float4x4 CurInvVP;
    row_major float4x4 LastFrameVP;
    float GaussianWeight;
    float MaxVelocity;
    uint SampleCount;
    float2 MotionBlurLastVPCB_padding;
};
cbuffer ViewportConstants : register(b10)
{
    float4 ViewportRect;
    float4 ScreenSize;
}

float GetGaussian(float dis)
{
    return exp(-dis * dis / (2 * GaussianWeight * GaussianWeight));
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    uint TexWidth = 0, TexHeight = 0;
    g_SceneColor.GetDimensions(TexWidth, TexHeight);
    float2 InvTexSize = float2(1.0f / TexWidth, 1.0f / TexHeight);
    float2 uv = input.position.xy * InvTexSize;

    float2 Velocity = (g_VelocityTex.Sample(g_PointClampSample, uv).rg * 2 - 1) * MaxVelocity;
    float2 VelocityDir = normalize(Velocity);
    float VelocityLength = length(Velocity);

    float3 TotalColor = float3(0, 0, 0);
    float TotalWeight = 0;
    for (int i = -SampleCount; i <= SampleCount; i++)
    {
        float t = float(i) / float(SampleCount * 2); // -SampleCount ~ SampleCount => -0.5 ~ 0.5
        float2 CurUV = uv + t * Velocity;
        float CurGaussian = GetGaussian(i);
        float3 CurColor = g_SceneColor.Sample(g_LinearClampSample, CurUV).rgb;
        TotalColor += CurColor * CurGaussian;
        TotalWeight += CurGaussian;
    }

    return float4(TotalColor / TotalWeight, 1);
}
