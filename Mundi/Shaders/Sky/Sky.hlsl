/**
 * Sky.hlsl
 * 절차적 스카이 스피어 렌더링 셰이더
 */

// --- Constant Buffers ---
cbuffer ModelBuffer : register(b0)
{
    matrix Model;
    matrix ModelInverseTranspose;
};

cbuffer ViewProjBuffer : register(b1)
{
    matrix View;
    matrix Proj;
    matrix InvView;
    matrix InvProj;
};

cbuffer ColorBuffer : register(b3)
{
    float4 InstanceColor;
    uint UUID;
    float3 Padding_Color;
};

cbuffer SkyParams : register(b9)
{
    float4 ZenithColor;
    float4 HorizonColor;
    float4 GroundColor;

    float3 SunDirection;
    float SunDiskSize;

    float4 SunColor;

    float HorizonFalloff;
    float SunHeight;
    float OverallBrightness;
    float CloudOpacity;
};

// --- Vertex Shader Input/Output ---
struct VSInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float3 WorldPos : TEXCOORD0;
    float3 LocalDir : TEXCOORD1;
};

struct PSOutput
{
    float4 Color : SV_Target0;
    uint UUID : SV_Target1;
};

// --- Vertex Shader ---
PSInput mainVS(VSInput Input)
{
    PSInput Output;

    // 월드 공간으로 변환
    float4 WorldPos = mul(float4(Input.Position, 1.0f), Model);
    Output.WorldPos = WorldPos.xyz;

    // 뷰-프로젝션 변환
    float4 ViewPos = mul(WorldPos, View);
    Output.Position = mul(ViewPos, Proj);

    // 스카이 샘플링용 방향 벡터 (월드 공간 방향, w=0으로 Translation 제거)
    float3 WorldDir = mul(float4(Input.Position, 0.0f), Model).xyz;
    Output.LocalDir = normalize(WorldDir);

    return Output;
}

// --- Pixel Shader ---
PSOutput mainPS(PSInput Input)
{
    // 정규화된 방향 벡터 (로컬 위치 기반)
    float3 ViewDir = normalize(Input.LocalDir);

    // 수직 성분: YUpToZUp 변환으로 인해 원래 Z축이 Y축으로 매핑됨
    // 원래 월드 Z축(위/아래) = 변환 후 Y축
    float Height = ViewDir.y;

    // 지평선 블렌딩 팩터 계산
    float HorizonBlend = saturate(pow(1.0f - abs(Height), HorizonFalloff));

    float3 SkyColor;
    if (Height > 0.0f)
    {
        // 상공: 천정(Zenith)과 지평선(Horizon) 블렌딩
        SkyColor = lerp(ZenithColor.rgb, HorizonColor.rgb, HorizonBlend);
    }
    else
    {
        // 하늘 아래: 지평선(Horizon)과 지면(Ground) 블렌딩
        SkyColor = lerp(GroundColor.rgb, HorizonColor.rgb, HorizonBlend);
    }

    // 태양 디스크 렌더링
    float3 SunDir = normalize(SunDirection);
    float SunAlignment = dot(ViewDir, SunDir);

    // 태양 디스크 강도 (SunDiskSize가 작을수록 더 작은 태양)
    float SunDisk = saturate((SunAlignment - (1.0f - SunDiskSize)) / SunDiskSize);
    SunDisk = pow(SunDisk, 2.0f);  // 더 선명한 경계를 위해 제곱

    // 태양 광채 (Glow)
    float SunGlow = saturate(pow(SunAlignment, 16.0f)) * 0.5f;

    // 태양 색상 블렌딩 (SunColor.a는 강도)
    float3 SunContribution = (SunDisk + SunGlow) * SunColor.rgb * SunColor.a;

    // 최종 색상 합성
    float3 FinalColor = SkyColor + SunContribution;

    // 전체 밝기 조절
    FinalColor *= OverallBrightness;

    PSOutput Output;
    Output.Color = float4(FinalColor, 1.0f);
    Output.UUID = UUID;
    return Output;
}
