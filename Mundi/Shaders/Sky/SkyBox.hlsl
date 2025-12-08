cbuffer ViewProjBuffer : register(b1)
{
    row_major float4x4 ViewMatrix;
    row_major float4x4 ProjectionMatrix;
    row_major float4x4 InverseViewMatrix;
    row_major float4x4 InverseProjectionMatrix;
};

// Cube map and sampler
TextureCube SkyCube : register(t0);
SamplerState LinearSampler : register(s0);

// Input vertex: POSITION only (float3)
struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Normal : NORMAL0;
    float2 TexCoord : TEXCOORD0;
    float4 Tangent : TANGENT0;
    float4 Color : COLOR;
};

struct PS_INPUT
{
    float4 PosH : SV_POSITION;
    float3 Dir : TEXCOORD0; // direction used to sample cubemap
};

// Vertex Shader
PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT o;

    // We want rotation only (no translation) so use w = 0 when multiplying by view.
    // Using float4(input.Pos, 0) applies rotation/scale but ignores view translation.
    float4 viewPos = mul(float4(input.Position, 0.0f), ViewMatrix);      // rotate vector by view (no translation)
    float4 projPos = mul(viewPos, ProjectionMatrix);                // project to clip space

    o.PosH = projPos.xyww;
    o.Dir = input.Position; // use model-space position as direction; if cube vertices are unit cube centered at origin, this works.

    return o;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    // direction should be normalized for proper sampling
    float3 dir = normalize(input.Dir.yzx);

    // Sample the cubemap; assume cubemap uses same coordinate convention as your cube verts.
    float4 color = SkyCube.Sample(LinearSampler, dir);

    color.r = pow(color.r, 2.2f);
    color.g = pow(color.g, 2.2f);
    color.b = pow(color.b, 2.2f);
    // No tonemapping here — apply later in your postprocess if needed.
    return float4(color.rgb,1);
}