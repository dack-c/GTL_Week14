# Depth of Field (DOF) 구현 상세 문서

## 목차
1. [개요](#1-개요)
2. [파이프라인 구조](#2-파이프라인-구조)
3. [각 Pass 상세](#3-각-pass-상세)
4. [핵심 알고리즘](#4-핵심-알고리즘)
5. [데이터 흐름](#5-데이터-흐름)
6. [파라미터 설명](#6-파라미터-설명)
7. [최적화 기법](#7-최적화-기법)
8. [알려진 이슈 및 해결](#8-알려진-이슈-및-해결)

---

## 1. 개요

### 1.1 DOF란?
Depth of Field(피사계 심도)는 카메라 렌즈의 물리적 특성을 시뮬레이션하여, 초점 거리에서 벗어난 물체를 흐리게 표현하는 포스트 프로세싱 효과입니다.

### 1.2 구현 방식
본 엔진은 **Hybrid DOF** 방식을 사용합니다:
- **Gather**: 각 픽셀이 주변을 샘플링하여 블러 (메인)
- **Scatter**: 밝은 Near 픽셀을 Point Sprite로 확장 (보조)

```
┌─────────────────────────────────────────────────────────────┐
│                      DOF 개념도                              │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│   Near Field        Focal Region        Far Field          │
│   (전경 블러)        (선명 영역)         (원경 블러)         │
│                                                             │
│   ░░░░░░░░░░       ████████████       ▒▒▒▒▒▒▒▒▒▒          │
│   ░ 카메라에 ░      █ 초점 거리 █       ▒ 배경/원경 ▒         │
│   ░  가까움  ░      █   근처   █       ▒          ▒         │
│   ░░░░░░░░░░       ████████████       ▒▒▒▒▒▒▒▒▒▒          │
│                                                             │
│   CoC < 0          CoC = 0            CoC > 0              │
│   (음수)            (제로)              (양수)               │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### 1.3 Circle of Confusion (CoC)
CoC는 각 픽셀의 "흐림 정도"를 나타내는 값입니다:
- **CoC = 0**: 완전히 선명 (초점 영역)
- **CoC < 0**: Near blur (전경, 카메라에 가까움)
- **CoC > 0**: Far blur (원경, 카메라에서 멂)

본 구현에서 CoC는 **0~1로 정규화**되어 저장됩니다.

---

## 2. 파이프라인 구조

### 2.1 전체 흐름

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         DOF Pipeline                                     │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│  [SceneColor]  [SceneDepth]                                             │
│       │             │                                                   │
│       └──────┬──────┘                                                   │
│              ▼                                                          │
│  ┌─────────────────────┐                                                │
│  │   1. Setup Pass     │  Full Res → Half Res                          │
│  │   (다운샘플 + CoC)   │  Near/Far 분리                                │
│  └──────────┬──────────┘                                                │
│             │                                                           │
│     ┌───────┴───────┐                                                   │
│     ▼               ▼                                                   │
│  [Far Field]    [Near Field]     (Half Res, RGBA16F)                   │
│     │               │                                                   │
│     │               ▼                                                   │
│     │    ┌─────────────────────┐                                        │
│     │    │  2. Tile Flatten    │  8x8 타일별 Min/Max CoC               │
│     │    └──────────┬──────────┘                                        │
│     │               ▼                                                   │
│     │    ┌─────────────────────┐                                        │
│     │    │  3. Tile Dilate     │  이웃 타일로 CoC 확장                  │
│     │    └──────────┬──────────┘                                        │
│     │               │                                                   │
│     │         [Tile Texture]                                            │
│     │               │                                                   │
│     ▼               ▼                                                   │
│  ┌─────────────────────┐                                                │
│  │   4. Blur Pass      │  Ring-based 2D Gather                         │
│  │   (Far → Near)      │  Scatter-as-Gather                            │
│  └──────────┬──────────┘                                                │
│             │                                                           │
│     ┌───────┴───────┐                                                   │
│     ▼               ▼                                                   │
│  [Blurred Far]  [Blurred Near]                                          │
│     │               │                                                   │
│     │               ▼                                                   │
│     │    ┌─────────────────────┐                                        │
│     │    │  5. Scatter Pass    │  Near 엣지 확장 (Point Sprite)        │
│     │    └──────────┬──────────┘                                        │
│     │               │                                                   │
│     │         [Scatter Tex]                                             │
│     │               │                                                   │
│     └───────┬───────┘                                                   │
│             ▼                                                           │
│  ┌─────────────────────┐                                                │
│  │   6. Recombine      │  Far → Sharp → Near 레이어 합성               │
│  └──────────┬──────────┘                                                │
│             ▼                                                           │
│      [Final Output]                                                     │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

### 2.2 해상도

| Pass | 입력 해상도 | 출력 해상도 |
|------|------------|------------|
| Setup | Full Res | Half Res |
| Tile Flatten | Half Res | Half Res / 8 |
| Tile Dilate | Tile Res | Tile Res |
| Blur | Half Res | Half Res |
| Scatter | Half Res | Half Res |
| Recombine | Full + Half | Full Res |

---

## 3. 각 Pass 상세

### 3.1 Setup Pass

**파일:**
- `DOFSetupPass.cpp`
- `DOF_Setup_PS.hlsl`

**목적:** Scene을 다운샘플하고 Near/Far Field로 분리

**입력:**
- `g_SceneColorTex` (Full Res)
- `g_SceneDepthTex` (Full Res)

**출력:** (MRT, Half Res)
- `SV_Target0`: Far Field (RGB = color, A = CoC)
- `SV_Target1`: Near Field (RGB = color, A = CoC)

**알고리즘:**
```hlsl
// 1. Bilateral 다운샘플 (깊이 기반 엣지 보존)
BilateralDownsampleResult dsResult = BilateralDownsampleWithCoC(...);

// 2. Sky Focus 처리 (하늘 블러 점진적 감소)
if (linearDepth > skyFocusStart && CoC > 0.0)
{
    float skyFalloff = 1.0 - smoothstep(skyFocusStart, skyFocusEnd, linearDepth);
    CoC *= skyFalloff;
}

// 3. Near/Far 분리
if (CoC > 0.0)      // Far
    output.FarField = float4(color, CoC);
else if (CoC < 0.0) // Near
    output.NearField = float4(color, -CoC);
else                // Focus
    // 둘 다 출력 안 함
```

**RTV Clear Color:**
```cpp
float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.01f };
// Alpha = 0.01로 Near 엣지 fade-out 지원
```

---

### 3.2 Tile Flatten Pass

**파일:**
- `DOFTilePass.cpp`
- `DOF_TileFlatten_CS.hlsl`

**목적:** 8x8 타일별 Min/Max CoC 계산

**입력:**
- Far Field 텍스처
- Near Field 텍스처

**출력:**
- Tile 텍스처 (RGBA16F)
  - R: Far Max CoC
  - G: Near Max CoC
  - B: Far Min CoC
  - A: Near Min CoC

**알고리즘:**
```hlsl
[numthreads(8, 8, 1)]
void mainCS(uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
    // 1. Far/Near CoC 읽기
    float farCoc = g_FarFieldTex.Load(int3(DTid.xy, 0)).a;
    float nearCoc = g_NearFieldTex.Load(int3(DTid.xy, 0)).a;

    // 2. Shared Memory에 저장
    s_FarCoC[GI] = farCoc;
    s_NearCoC[GI] = nearCoc;
    GroupMemoryBarrierWithGroupSync();

    // 3. Parallel Reduction으로 Min/Max 계산
    for (uint stride = 32; stride > 0; stride >>= 1)
    {
        if (GI < stride)
        {
            s_FarCoC[GI] = max(s_FarCoC[GI], s_FarCoC[GI + stride]);
            s_NearCoC[GI] = max(s_NearCoC[GI], s_NearCoC[GI + stride]);
            // Min도 동일하게...
        }
        GroupMemoryBarrierWithGroupSync();
    }

    // 4. 타일 대표가 출력
    if (GI == 0)
    {
        g_OutputTile[tileCoord] = float4(farMax, nearMax, farMin, nearMin);
    }
}
```

---

### 3.3 Tile Dilate Pass

**파일:**
- `DOFTilePass.cpp`
- `DOF_TileDilate_CS.hlsl`

**목적:** 인접 타일로 CoC 확장 (Scatter-as-Gather 지원)

**알고리즘:**
```hlsl
// 3x3 이웃 타일에서 Max CoC 수집
float maxFarCoc = 0;
float maxNearCoc = 0;

for (int dy = -1; dy <= 1; dy++)
{
    for (int dx = -1; dx <= 1; dx++)
    {
        float4 neighborTile = g_TileTex.Load(int3(tileCoord + int2(dx, dy), 0));
        maxFarCoc = max(maxFarCoc, neighborTile.x);
        maxNearCoc = max(maxNearCoc, neighborTile.y);
    }
}
```

**Dilate가 필요한 이유:**
```
┌─────────────────────────────────────────────────────────────┐
│ 작은 밝은 점 (1픽셀 가로등) 문제                             │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│   Without Dilate:                                           │
│   ┌───┬───┬───┐                                            │
│   │0.0│0.0│0.0│  ← 빈 타일들                               │
│   ├───┼───┼───┤                                            │
│   │0.0│0.8│0.0│  ← 가로등 타일만 높은 CoC                  │
│   ├───┼───┼───┤                                            │
│   │0.0│0.0│0.0│                                            │
│   └───┴───┴───┘                                            │
│   → 주변 픽셀이 가로등을 샘플링하지 못함                     │
│                                                             │
│   With Dilate:                                              │
│   ┌───┬───┬───┐                                            │
│   │0.8│0.8│0.8│  ← Dilate로 확장                          │
│   ├───┼───┼───┤                                            │
│   │0.8│0.8│0.8│                                            │
│   ├───┼───┼───┤                                            │
│   │0.8│0.8│0.8│                                            │
│   └───┴───┴───┘                                            │
│   → 주변 픽셀이 "여기 밝은 것 있음" 힌트를 받음              │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

### 3.4 Blur Pass

**파일:**
- `DOFBlurPass.cpp`
- `DOF_Blur_PS.hlsl`

**목적:** Ring-based 2D Gather로 원형 보케 생성

**입력:**
- Far/Near Field 텍스처
- Dilated Tile 텍스처

**출력:**
- Blurred Far/Near Field

**알고리즘 (Ring Sampling):**
```hlsl
// Ring 설정: 1 + 8 + 16 + 24 + 32 + 40 = 121 샘플
const int MAX_RING = 5;

// Ring 0: 중심 (1 샘플)
{
    float weight = centerCoc + 0.01;
    accColor += centerColor * weight;
    accCoc += centerCoc * weight;
    accWeight += weight;
}

// Ring 1~5: 원형 샘플링
for (int ring = 1; ring <= MAX_RING; ring++)
{
    float ringRadius = pixelRadius * (ring / MAX_RING);
    int sampleCount = ring * 8;
    float angleStep = 2π / sampleCount;

    for (int s = 0; s < sampleCount; s++)
    {
        float angle = s * angleStep;
        float2 offset = float2(cos(angle), sin(angle)) * ringRadius;

        // Scatter-as-Gather: 샘플의 CoC가 여기까지 도달하는지 검사
        float coverage = saturate((sampleRadius - distance + 1) / sampleRadius);
        float weight = coverage * distanceFalloff * sampleCoc;

        accColor += sampleColor * weight;
        accCoc += sampleCoc * weight;
        accWeight += weight;
    }
}

output.Color = float4(accColor / accWeight, accCoc / accWeight);
```

**샘플링 패턴:**
```
        Ring 5 (40 샘플)
       ·  ·  ·  ·  ·  ·
      ·              ·
     ·    Ring 3     ·
    ·   ·  ·  ·  ·    ·
   ·  ·          ·  ·
  · ·    Ring 1    · ·
  · ·   ·  ·  ·   · ·
  · ·  ·  [C]  ·  · ·   [C] = Center
  · ·   ·  ·  ·   · ·
  · ·    Ring 2    · ·
   ·  ·          ·  ·
    ·   ·  ·  ·  ·    ·
     ·    Ring 4     ·
      ·              ·
       ·  ·  ·  ·  ·  ·
```

---

### 3.5 Scatter Pass

**파일:**
- `DOFScatterPass.cpp`
- `DOF_Scatter_VS.hlsl`
- `DOF_Scatter_PS.hlsl`

**목적:** 큰 CoC를 가진 Near 픽셀을 Point Sprite로 확장 (엣지 bleeding)

**왜 Scatter가 필요한가:**
```
┌─────────────────────────────────────────────────────────────┐
│ Gather의 한계                                                │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│   [Near Object]  │  [Focus Area]                           │
│      ████████    │                                          │
│      ████████    │    Gather 방향                           │
│      ████████ ◀──┼─── (Focus가 Near를 샘플링)               │
│      ████████    │                                          │
│                  │                                          │
│   Near의 블러가 Focus 위로 번져야 하는데,                    │
│   Focus는 CoC=0이라 샘플링을 안 함!                          │
│                                                             │
│   → Scatter로 Near가 직접 Focus 위에 그려져야 함             │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

**Vertex Shader:**
```hlsl
VS_OUTPUT mainVS(VS_INPUT input)
{
    // InstanceID로 픽셀 좌표 계산
    uint pixelIndex = input.InstanceID + PixelOffset;
    uint pixelX = pixelIndex % ScreenSize.x;
    uint pixelY = pixelIndex / ScreenSize.x;

    // 텍스처에서 색상/CoC 읽기
    float4 texData = g_NearFieldTex.Load(int3(pixelX, pixelY, 0));
    float coc = texData.a;

    // 조건: 큰 CoC만 Scatter
    float cocRadiusPx = coc * MaxBlurRadius;
    bool shouldScatter = (coc > CocThreshold) && (cocRadiusPx > 3.0);

    if (!shouldScatter)
    {
        // NDC 범위 밖으로 → 렌더링 안 됨
        output.Position = float4(10.0, 10.0, 0.0, 1.0);
        return output;
    }

    // 쿼드 확장
    float2 centerNDC = pixelUV * 2.0 - 1.0;
    float2 radiusNDC = cocRadiusPx * TexelSize * 2.0;
    output.Position = float4(centerNDC + QuadOffsets[vertexID] * radiusNDC, 0.5, 1.0);
    output.Color = texData.rgb * BokehIntensity;
    output.CocRadius = cocRadiusPx;

    return output;
}
```

**Pixel Shader:**
```hlsl
PS_OUTPUT mainPS(PS_INPUT input)
{
    // 원형 마스크
    float2 uv = input.TexCoord * 2.0 - 1.0;
    float dist = length(uv);
    if (dist > 1.0) discard;

    // 부드러운 엣지
    float alpha = 1.0 - smoothstep(0.9, 1.0, dist);

    // 면적 정규화 (에너지 보존)
    float area = 3.14159 * input.CocRadius * input.CocRadius;
    float areaNorm = 1.0 / max(area, 1.0);

    output.Color.rgb = input.Color * alpha * areaNorm;
    output.Color.a = alpha * areaNorm;

    return output;
}
```

**배치 처리:**
```cpp
// GPU TDR 방지를 위한 배치 처리
UINT MAX_QUADS_PER_BATCH = 65536;

for (UINT startPixel = 0; startPixel < totalPixels; startPixel += MAX_QUADS_PER_BATCH)
{
    // PixelOffset 업데이트 (SV_InstanceID는 항상 0부터 시작하므로)
    cb->PixelOffset = startPixel;
    DeviceContext->DrawInstanced(4, quadCount, 0, 0);
}
```

---

### 3.6 Recombine Pass

**파일:**
- `DOFRecombinePass.cpp`
- `DOF_Recombine_PS.hlsl`

**목적:** Far, Sharp, Near 레이어를 합성하여 최종 결과 생성

**입력:**
- `g_SceneColorTex` (Full Res, Sharp)
- `g_FarFieldTex` (Half Res, Blurred)
- `g_NearFieldTex` (Half Res, Blurred)
- `g_ScatterTex` (Half Res, Near Scatter)

**합성 순서:**
```
┌─────────────────────────────────────────────────────────────┐
│ Layer 합성 순서 (Back to Front)                             │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│   Layer 1: Far (배경)                                       │
│   ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒           │
│   │                                                         │
│   ▼ lerp(scene, far, farMask)                              │
│                                                             │
│   Layer 2: Sharp (초점)                                     │
│   ▒▒▒▒▒▒▒▒████████████████████████▒▒▒▒▒▒▒▒                │
│   │                                                         │
│   ▼ (이미 scene에 포함)                                     │
│                                                             │
│   Layer 3: Near (전경)                                      │
│   ░░░░░░░░░░░░▒▒▒▒████████████████████████                 │
│   │                                                         │
│   ▼ lerp(current, near, nearMask)                          │
│                                                             │
│   Final Output                                              │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

**알고리즘:**
```hlsl
PS_OUTPUT mainPS(PS_INPUT input)
{
    float4 sceneColor = g_SceneColorTex.Sample(...);
    float4 farField = BilateralUpsample(g_FarFieldTex, ...);
    float4 nearField = BilateralUpsample(g_NearFieldTex, ...);
    float4 scatter = g_ScatterTex.Sample(...);

    float4 finalColor = sceneColor;

    // Layer 1: Far Blur
    if (CoC > 0.001)
    {
        float farMask = smoothstep(CoC);
        finalColor = lerp(finalColor, farField, farMask);
    }

    // Layer 2: Near Blur + Scatter
    float nearMask = nearField.a;
    float3 nearWithScatter = nearField.rgb;

    if (scatter.a > 0.001)
    {
        // Scatter는 Near가 약한 곳에서만 기여
        float scatterOnly = saturate(scatter.a - nearMask * 0.5);
        float3 scatterColor = scatter.rgb / max(scatter.a, 0.001);
        nearWithScatter = lerp(nearField.rgb, scatterColor, scatterOnly);
        nearMask = max(nearMask, scatter.a);
    }

    nearMask = smoothstep(nearMask);
    finalColor = lerp(finalColor, nearWithScatter, nearMask);

    return finalColor;
}
```

---

## 4. 핵심 알고리즘

### 4.1 CoC 계산

```hlsl
float CalculateCoC(float viewDepth, float focalDistance, float focalRegion,
                   float nearTransition, float farTransition, ...)
{
    float focalStart = focalDistance - focalRegion * 0.5;
    float focalEnd = focalDistance + focalRegion * 0.5;

    // 초점 영역 내부 = 선명
    if (viewDepth >= focalStart && viewDepth <= focalEnd)
        return 0.0;

    if (viewDepth < focalStart)
    {
        // Near: 음수 CoC
        float distance = focalStart - viewDepth;
        return -saturate(distance / nearTransition);
    }
    else
    {
        // Far: 양수 CoC
        float distance = viewDepth - focalEnd;
        return saturate(distance / farTransition);
    }
}
```

**시각화:**
```
CoC
 1.0 ─┐                              ┌─────────
      │                              │
      │                              │
 0.0 ─┼──────────────────────────────┼─────────
      │              ████            │
      │         █████    █████       │
-1.0 ─┼────█████              █████──┤
      │                              │
      └──────────────────────────────┴─────────
          Near    Focal   Focal    Far      Depth
         Trans    Start    End    Trans
```

### 4.2 Bilateral Upsampling

엣지 보존을 위한 깊이 기반 업샘플링:

```hlsl
float4 BilateralUpsample(Texture2D blurTex, float2 uv, float centerDepth, float2 texelSize)
{
    float4 result = 0;
    float totalWeight = 0;

    for (int y = -1; y <= 1; y++)
    {
        for (int x = -1; x <= 1; x++)
        {
            float2 sampleUV = uv + float2(x, y) * texelSize;
            float4 sample = blurTex.Sample(linearSampler, sampleUV);

            float sampleDepth = depthTex.Sample(pointSampler, sampleUV).r;
            sampleDepth = LinearizeDepth(sampleDepth, ...);

            // 깊이 차이 기반 가중치
            float depthDiff = abs(sampleDepth - centerDepth) / max(centerDepth, 0.1);
            float weight = exp(-depthDiff * 2.0);

            result += sample * weight;
            totalWeight += weight;
        }
    }

    return result / max(totalWeight, 0.001);
}
```

### 4.3 Scatter-as-Gather

Tile 정보를 활용하여 Gather에서 Scatter 효과 구현:

```hlsl
// Tile에서 주변 최대 CoC 가져오기
float tileMaxCoc = g_TileTex.Load(tileCoord).x;

// 검색 반경 결정: 자신의 CoC와 타일 Max 중 큰 값
float searchRadius = max(centerCoc, tileMaxCoc);

// 샘플링 시 coverage 검사
float sampleRadiusPx = sampleCoc * maxBlurRadius;
float coverage = saturate((sampleRadiusPx - distance + 1) / sampleRadiusPx);
```

---

## 5. 데이터 흐름

### 5.1 텍스처 포맷

| 텍스처 | 포맷 | 용도 |
|--------|------|------|
| DOF[0] | RGBA16F | Far Field (color + CoC) |
| DOF[1] | RGBA16F | Near Field (color + CoC) |
| DOF[2] | RGBA16F | Temp (Blur 중간 결과) |
| DOF[3] | RGBA16F | Temp (미사용) |
| Tile | RGBA16F | Min/Max CoC per tile |
| Scatter | RGBA16F | Near Scatter 결과 |

### 5.2 Constant Buffer

```cpp
// DOF Setup/Recombine CB
struct DOFConstants
{
    float FocalDistance;        // m
    float FocalRegion;          // m
    float NearTransitionRegion; // m
    float FarTransitionRegion;  // m
    float MaxNearBlurSize;      // pixels
    float MaxFarBlurSize;       // pixels
    float NearClip;
    float FarClip;
    int   IsOrthographic;
};

// Scatter CB
struct ScatterConstants
{
    float2 TexelSize;
    float2 ScreenSize;
    float  MaxBlurRadius;
    float  CocThreshold;
    float  BokehIntensity;
    uint   PixelOffset;     // 배치 오프셋
};
```

---

## 6. 파라미터 설명

### 6.1 CamMod_DOF 파라미터

| 파라미터 | 단위 | 기본값 | 설명 |
|----------|------|--------|------|
| FocalDistance | m | 6.0 | 초점 거리 |
| FocalRegion | m | 4.0 | 완전 선명 영역 크기 |
| NearTransitionRegion | m | 5.0 | Near blur 전환 거리 |
| FarTransitionRegion | m | 20.0 | Far blur 전환 거리 |
| MaxNearBlurSize | px | 32.0 | Near 최대 블러 반경 |
| MaxFarBlurSize | px | 32.0 | Far 최대 블러 반경 |

### 6.2 파라미터 시각화

```
                    FocalDistance
                         │
                         ▼
    ◄─── NearTrans ───►│◄─ FocalRegion ─►│◄─── FarTrans ────►
                        │                 │
   ▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒░░░│█████████████████│░░░▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓
   MaxNearBlur         0                 0          MaxFarBlur
                       (선명)            (선명)
```

---

## 7. 최적화 기법

### 7.1 Half Resolution
- Setup에서 1/2 해상도로 다운샘플
- 블러 연산량 1/4로 감소
- Recombine에서 Bilateral Upsample로 품질 유지

### 7.2 Tile-based Culling
- 8x8 타일별 Min/Max CoC 저장
- CoC가 0인 타일은 블러 스킵 가능
- Early-out으로 불필요한 샘플링 방지

### 7.3 Ring Sampling
- 균일한 원형 분포로 보케 품질 향상
- 121 샘플로 충분한 품질 (조절 가능)
- Loop unroll로 성능 최적화

### 7.4 Scatter 배치 처리
- 65536 쿼드씩 배치 처리
- GPU TDR (Timeout Detection Recovery) 방지
- PixelOffset으로 SV_InstanceID 보정

---

## 8. 알려진 이슈 및 해결

### 8.1 Near 엣지 Fade-out

**문제:** Near 오브젝트가 빈 공간과 겹칠 때 엣지가 solid하게 유지됨

**원인:** 빈 공간의 CoC=0이라 weight=0 → Near를 희석하지 못함

**해결:** Setup RTV Clear Color의 Alpha를 0.01로 설정
```cpp
float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.01f };
```

### 8.2 Sky 블러 문제

**문제:** 하늘이 최대 Far blur를 받아 너무 흐림

**해결:** Sky Focus Distance로 점진적 감소
```hlsl
const float skyFocusStart = FarClip * 0.5;
const float skyFocusEnd = FarClip * 0.9;

if (linearDepth > skyFocusStart && CoC > 0.0)
{
    float skyFalloff = 1.0 - smoothstep(skyFocusStart, skyFocusEnd, linearDepth);
    CoC *= skyFalloff;
}
```

### 8.3 Scatter 정점 순서

**문제:** Triangle Strip 쿼드가 백페이스 컬링됨

**해결:** CW 순서로 정점 배치
```hlsl
// 0: 좌하, 1: 좌상, 2: 우하, 3: 우상
static const float2 QuadOffsets[4] = {
    float2(-1, -1),
    float2(-1,  1),
    float2( 1, -1),
    float2( 1,  1)
};
```

### 8.4 SV_InstanceID 배치 오프셋

**문제:** DrawInstanced의 StartInstanceLocation이 SV_InstanceID에 반영 안 됨

**해결:** Constant Buffer로 PixelOffset 전달
```hlsl
cbuffer ScatterCB { uint PixelOffset; };
uint pixelIndex = input.InstanceID + PixelOffset;
```

### 8.5 면적 정규화

**문제:** 큰 보케가 더 밝아지는 현상

**해결:** 면적(πr²)으로 정규화
```hlsl
float area = 3.14159 * cocRadius * cocRadius;
float areaNorm = 1.0 / max(area, 1.0);
output.Color = color * areaNorm;
```

---

## 부록: 파일 목록

### C++ 소스
```
Mundi/Source/Runtime/Renderer/PostProcessing/
├── DOFSetupPass.cpp/.h
├── DOFTilePass.cpp/.h
├── DOFBlurPass.cpp/.h
├── DOFScatterPass.cpp/.h
└── DOFRecombinePass.cpp/.h
```

### 셰이더
```
Mundi/Shaders/PostProcess/
├── DOF_Common.hlsli
├── DOF_Setup_PS.hlsl
├── DOF_TileFlatten_CS.hlsl
├── DOF_TileDilate_CS.hlsl
├── DOF_Blur_PS.hlsl
├── DOF_Scatter_VS.hlsl
├── DOF_Scatter_PS.hlsl
└── DOF_Recombine_PS.hlsl
```

### Camera Modifier
```
Mundi/Source/Runtime/Engine/GameFramework/Camera/
└── CamMod_DOF.h
```

---

*문서 작성일: 2024*
*작성자: Claude Code*
