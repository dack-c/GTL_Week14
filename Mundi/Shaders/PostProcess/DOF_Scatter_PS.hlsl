// DOF Scatter Pixel Shader
// 보케 모양 렌더링 (원형)

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;   // Bokeh 내 UV (0~1)
    float3 Color    : TEXCOORD1;
    float  CocRadius: TEXCOORD2;
};

struct PS_OUTPUT
{
    float4 Color : SV_Target0;
};

PS_OUTPUT mainPS(PS_INPUT input)
{
    PS_OUTPUT output;

    // UV를 -1~1로 변환
    float2 uv = input.TexCoord * 2.0 - 1.0;

    // 원형 마스크
    float dist = length(uv);

    // 원 바깥은 버림
    if (dist > 1.0)
    {
        discard;
    }

    // 부드러운 엣지 (안티앨리어싱)
    float edgeSoftness = 0.1;
    float alpha = 1.0 - smoothstep(1.0 - edgeSoftness, 1.0, dist);

    // 보케 내부 밝기 분포 (Disk Blur - 균일)
    // 옵션: Gaussian이나 Ring 형태로 변경 가능
    float intensity = alpha;

    // 면적 정규화: 큰 보케일수록 개별 픽셀 기여도 낮춤
    // 면적 = π * r² → 정규화 = 1 / (π * r²)
    float area = 3.14159 * input.CocRadius * input.CocRadius;
    float areaNorm = 1.0 / max(area, 1.0);

    output.Color.rgb = input.Color * intensity * areaNorm;
    output.Color.a = intensity * areaNorm;

    return output;
}
