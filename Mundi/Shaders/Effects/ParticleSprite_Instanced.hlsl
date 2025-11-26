// GPU instanced particle sprite shader.
// Matches ParticleSprite.hlsl behaviour but reads per-instance data from a structured buffer
// so that CPU uploads only one record per sprite.

// b0: ModelBuffer (kept for interface parity ? values unused for billboards)
cbuffer ModelBuffer : register(b0)
{
	row_major float4x4 WorldMatrix;
	row_major float4x4 WorldInverseTranspose;
};

// b1: View/Projection like the non-instanced version
cbuffer ViewProjBuffer : register(b1)
{
	row_major float4x4 ViewMatrix;
	row_major float4x4 ProjectionMatrix;
	row_major float4x4 InverseViewMatrix;
	row_major float4x4 InverseProjectionMatrix;
};

// b2: SubUV Parameters
cbuffer SubUVBuffer : register(b2)
{
	int SubImages_Horizontal;  // NX
	int SubImages_Vertical;    // NY
	int InterpMethod;          // 0=None, 1=LinearBlend
	float Padding0;            // 16바이트 정렬
};

// b3: Particle Required Parameters
cbuffer SubUVBuffer : register(b3)
{
	uint ScreenAlignment;   // 0: PSA_Square, 1: PSA_Velocity
	float3 Padding;         // 16바이트 정렬용 패딩
};


Texture2D ParticleTex : register(t0);
SamplerState ParticleSampler : register(s0);

// Structured buffer carrying FParticleInstanceData written on the CPU side.
struct FParticleInstanceData
{
	float3 Position;
	float2 Size;
	float4 Color;
	float Rotation;
	float3 Velocity;
};

StructuredBuffer<FParticleInstanceData> ParticleInstances : register(t14);

struct VSInput
{
	uint VertexID : SV_VertexID;
	uint InstanceID : SV_InstanceID;
};

struct VSOutput
{
	float4 Position : SV_POSITION;
	float2 UV : TEXCOORD0;
	float4 Color : COLOR0;
};

static const uint CornerIndexLUT[6] =
{
    0, 1, 2,
	0, 3, 2
};

static const float2 CornerOffsets[4] = {
    float2(-1.0f, -1.0f), float2(1.0f, -1.0f),
	 float2(1.0f, 1.0f), float2(-1.0f, 1.0f)
};

VSOutput mainVS(VSInput In)
{
	VSOutput Out;

	FParticleInstanceData Instance = ParticleInstances[In.InstanceID];
    
    float3 CamRight = InverseViewMatrix[0].xyz;
    float3 CamUp    = InverseViewMatrix[1].xyz;
    float3 CamPos   = InverseViewMatrix[3].xyz;

    float3 FinalRight;
    float3 FinalUp;
	float2 finalSize = Instance.Size;
    if (ScreenAlignment == 1) // PSA_Velocity
    {
        float SpeedSq = dot(Instance.Velocity, Instance.Velocity);
        
        // 속도가 거의 0이면 깨짐 방지를 위해 카메라 정면 보기
        if (SpeedSq < 0.001f)
        {
            FinalRight = CamRight;
            FinalUp    = CamUp;
        }
        else
        {
        	float Speed = sqrt(SpeedSq);
            FinalUp = normalize(Instance.Velocity);
            float3 ToCam = normalize(CamPos - Instance.Position);
            FinalRight = normalize(cross(ToCam, FinalUp));
        	finalSize.y += (Speed * 0.2f);
        }
    }
    else // PSA_Square
    {
        float cosR = cos(Instance.Rotation);
        float sinR = sin(Instance.Rotation);
        FinalRight = CamRight * cosR - CamUp * sinR;
        FinalUp    = CamRight * sinR + CamUp * cosR;
    }

    // 코너 인덱스 가져오기
    uint cornerIndex = CornerIndexLUT[In.VertexID];
    float2 corner = CornerOffsets[cornerIndex];
    
	float2 halfSize = finalSize * 0.5f;
    float3 worldPos = Instance.Position
                    + (FinalRight * corner.x * halfSize.x)
                    + (FinalUp    * corner.y * halfSize.y);

    float4 viewPos = mul(float4(worldPos, 1.0f), ViewMatrix);
    Out.Position = mul(viewPos, ProjectionMatrix);
    
    Out.UV = float2(corner.x * 0.5f + 0.5f, -corner.y * 0.5f + 0.5f);
    Out.Color = Instance.Color;
    
    return Out;
}

float4 mainPS(VSOutput In) : SV_TARGET
{
	float4 tex = ParticleTex.Sample(ParticleSampler, In.UV);
	return tex * In.Color;
}
