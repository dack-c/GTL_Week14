

// b0: ModelBuffer (VS) - Matches ModelBufferType exactly (128 bytes)
cbuffer ModelBuffer : register(b0)
{
    row_major float4x4 WorldMatrix; // 64 bytes
    row_major float4x4 WorldInverseTranspose; // 64 bytes - For correct normal transformation
};

// b1: ViewProjBuffer (VS) - Matches ViewProjBufferType
cbuffer ViewProjBuffer : register(b1)
{
    row_major float4x4 ViewMatrix;
    row_major float4x4 ProjectionMatrix;
    row_major float4x4 InverseViewMatrix;
    row_major float4x4 InverseProjectionMatrix;
};

Texture2D ParticleTex : register(t0);
SamplerState ParticleSampler : register(s0);

struct VSInput
{
    float3 Position : POSITION;
    float2 Corner : TEXCOORD0;
    float2 Size : TEXCOORD1;
    float4 Color : COLOR0;
    float Rotation : TEXCOORD2;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
    float4 Color : COLOR0;
};

PSInput mainVS(VSInput In)
{
    PSInput Out;

    // 카메라 오른쪽/위 벡터를 ViewMatrix에서 추출
    // float3 Right = InverseViewMatrix[0].xyz;
    // float3 Up = InverseViewMatrix[1].xyz;
    float3 Right = normalize(float3(ViewMatrix._11, ViewMatrix._21, ViewMatrix._31));
    float3 Up = normalize(float3(ViewMatrix._12, ViewMatrix._22, ViewMatrix._32));

    float2 halfSize = In.Size * 0.5f;

    // Rotation 적용: Corner를 회전시킴
    float cosR = cos(In.Rotation);
    float sinR = sin(In.Rotation);
    float2 rotatedCorner;
    rotatedCorner.x = In.Corner.x * cosR - In.Corner.y * sinR;
    rotatedCorner.y = In.Corner.x * sinR + In.Corner.y * cosR;

    float3 worldPos =
        In.Position
        + Right * rotatedCorner.x * halfSize.x
        + Up * rotatedCorner.y * halfSize.y;

    float4 viewPos = mul(float4(worldPos, 1.0f), ViewMatrix);
    float4 projPos = mul(viewPos, ProjectionMatrix);

    Out.Position = projPos;
    Out.UV = In.Corner * 0.5f + 0.5f; // (-1~1) → (0~1)
    Out.Color = In.Color;

    return Out;
}

float4 mainPS(PSInput In) : SV_TARGET
{
    float4 tex = ParticleTex.Sample(ParticleSampler, In.UV);
    float4 color = tex * In.Color;

    return color;
}
