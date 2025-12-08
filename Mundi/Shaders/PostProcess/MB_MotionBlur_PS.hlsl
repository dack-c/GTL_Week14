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
    int SampleCount;
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

    float2 Velocity = (g_VelocityTex.Sample(g_PointClampSample, uv).rg * 2 - 1);
    Velocity.x = abs(Velocity.x) < 0.0005f ? 0 : Velocity.x;
    Velocity.y = abs(Velocity.y) < 0.0005f ? 0 : Velocity.y;
    Velocity *= MaxVelocity;
    float2 VelocityDir = normalize(Velocity);
    float VelocityLength = length(Velocity);

    float3 TotalColor = float3(0, 0, 0);
    float TotalWeight = 0;
    for (int i = -SampleCount; i <= SampleCount; i++)
    {
        float t = float(i) / float(SampleCount * 2); // -SampleCount ~ SampleCount => -0.5 ~ 0.5
        float2 CurUV = uv + i * Velocity;
        //float Weight = GetGaussian(i);
        float Weight = 1 - abs(t);
        float3 CurColor = g_SceneColor.Sample(g_LinearClampSample, CurUV).rgb;
        TotalColor += CurColor * Weight;
        TotalWeight += Weight;
    }
    //return float4(g_SceneColor.Sample(g_LinearClampSample, uv).rgb, 1);
    //return float4(g_VelocityTex.Sample(g_LinearClampSample, uv).rgb, 1);
    //return float4(abs(Velocity.rg), 0, 1);
    return float4(TotalColor / TotalWeight, 1);
}
//const int COUNT = 8;
//float BlurScale = 0.25f;
//float3 TotalColor = float3(0, 0, 0);
//float TotalWeight = 0;
//float VelocityLength = length(VelocityUV) * BlurScale;
////return float4(abs(VelocityUV), 0, 1);
//for (int i = -COUNT; i <= COUNT; i++)
//{
//    float dt = (float)i / COUNT;
//    float Weight = 1.0 - abs(dt);
//    float2 CurUV = uv + VelocityDir * i * VelocityLength;

//    TotalColor += g_SceneColor.Sample(g_LinearClampSample, CurUV).rgb * Weight;
//    TotalWeight += Weight;
//}

//return float4(TotalColor / TotalWeight, 1);