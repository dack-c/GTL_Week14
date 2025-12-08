#include "DOF_Common.hlsli"

// Input Textures (Near 또는 Far 분리 텍스처)
Texture2D    g_InputTex : register(t0);   // Near: DOF[0], Far: DOF[1]

SamplerState g_LinearClampSample : register(s0);
SamplerState g_PointClampSample : register(s1);

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

// Constant Buffer
cbuffer DOFBlurCB : register(b2)
{

}

cbuffer ViewportConstants : register(b10)
{
    float4 ViewportRect;
    float4 ScreenSize;
}

#define PI 3.14159265359
#define COC_THRESHOLD 0.001

struct PS_OUTPUT
{
    float4 Color : SV_Target0;
};

PS_OUTPUT mainPS(PS_INPUT input)
{

}
