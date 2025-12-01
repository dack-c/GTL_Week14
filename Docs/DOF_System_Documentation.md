# Mundi Engine - Depth of Field (DOF) System Documentation

## 개요

본 문서는 Mundi 엔진의 Cinematic DOF (Depth of Field) 시스템을 설명합니다.
언리얼 엔진 5의 Cinematic DOF를 참고하여 구현되었습니다.

---

## 파이프라인 아키텍처

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                          DOF Pipeline Overview                               │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│  [Full Resolution]                                                           │
│       │                                                                      │
│       ▼                                                                      │
│  ┌──────────┐     ┌──────────┐     ┌──────────┐                             │
│  │  Setup   │────▶│   Tile   │────▶│   Blur   │                             │
│  │   Pass   │     │   Pass   │     │   Pass   │                             │
│  └──────────┘     └──────────┘     └──────────┘                             │
│       │                │                │                                    │
│       │                │                ├──▶ DOF[1]: Near Blurred            │
│       │                │                └──▶ DOF[2]: Far Blurred             │
│       ▼                ▼                │                                    │
│   DOF[0]:          Tile SRV            ▼                                    │
│   RGB + CoC         (Dilated      ┌──────────┐                              │
│   (Half Res)         MaxCoC)      │ Scatter  │                              │
│                                   │   Pass   │                              │
│                                   └──────────┘                              │
│                                        │                                     │
│                                        ├──▶ DOF[1] += Near Scatter           │
│                                        └──▶ DOF[2] += Far Scatter            │
│                                        │                                     │
│                                        ▼                                     │
│  [Full Resolution]              ┌─────────────┐                             │
│  Scene Color ──────────────────▶│  Recombine  │──▶ Final Output             │
│  Scene Depth ──────────────────▶│    Pass     │                             │
│                                 └─────────────┘                             │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## 렌더 타겟 구조

| Index | Name | Resolution | Format | Description |
|-------|------|------------|--------|-------------|
| DOF[0] | Setup Output | Half Res | RGBA16F | RGB + CoC (Near: 음수, Far: 양수) |
| DOF[1] | Near Blurred | Half Res | RGBA16F | Near Blur + Scatter (Premultiplied Alpha) |
| DOF[2] | Far Blurred | Half Res | RGBA16F | Far Blur + Scatter (Premultiplied Alpha) |

---

## 1. Setup Pass

### 역할
- Full Resolution Scene Color를 Half Resolution으로 다운샘플링
- 각 픽셀의 Circle of Confusion (CoC) 계산
- Bilateral Downsampling으로 엣지 보존

### 입력
| Slot | Resource | Description |
|------|----------|-------------|
| t0 | SceneColor | Full Resolution Scene Color |
| t1 | SceneDepth | Full Resolution Depth Buffer |

### 출력
| Target | Content |
|--------|---------|
| DOF[0] | RGB: 다운샘플된 색상, A: CoC (부호 있음) |

### CoC 계산 공식

```hlsl
// DOF_Common.hlsli:26-65
float CalculateCoC(float viewDepth, float focalDistance, float focalRegion,
                   float nearTransition, float farTransition, ...)
{
    float focalStart = focalDistance - focalRegion * 0.5;
    float focalEnd = focalDistance + focalRegion * 0.5;

    // 초점 영역 내부 = 선명 (CoC = 0)
    if (viewDepth >= focalStart && viewDepth <= focalEnd)
        return 0.0;

    // Near Field (전경) - 음수 CoC
    if (viewDepth < focalStart)
    {
        float distance = focalStart - viewDepth;
        return -saturate(distance / nearTransition);  // -1 ~ 0
    }

    // Far Field (원경) - 양수 CoC
    float distance = viewDepth - focalEnd;
    return saturate(distance / farTransition);  // 0 ~ 1
}
```

### CoC 부호 규칙

| CoC 값 | 의미 | 레이어 |
|--------|------|--------|
| < 0 | Near Field (전경, 카메라 가까움) | Foreground |
| = 0 | In-Focus (초점 맞음) | Sharp |
| > 0 | Far Field (원경, 카메라 멀음) | Background |

### 파일 위치
- **셰이더**: `Mundi/Shaders/PostProcess/DOF_Setup_PS.hlsl`
- **C++ Pass**: `Mundi/Source/Runtime/Renderer/PostProcessing/DOFSetupPass.cpp`

---

## 2. Tile Pass

### 역할
- 8x8 타일 단위로 Max/Min CoC 계산
- 이웃 타일로 CoC 확장 (Dilate)
- Scatter-as-Gather 검색 범위 최적화

### 구성
1. **Flatten Pass**: 8x8 타일별 Max/Min CoC 계산
2. **Dilate Pass**: 이웃 타일로 Max CoC 확장

### Flatten Pass (Compute Shader)

```hlsl
// DOF_TileFlatten_CS.hlsl
[numthreads(8, 8, 1)]
void mainCS(uint3 GroupId, uint3 GroupThreadId, uint GroupIndex)
{
    // 픽셀의 CoC 절대값 읽기
    float cocAbs = abs(g_InputTex[pixelPos].a);

    // Shared memory에 저장
    sharedMaxCoC[GroupIndex] = cocAbs;
    sharedMinCoC[GroupIndex] = (cocAbs > 0.001) ? cocAbs : 999999.0;

    // Parallel reduction으로 타일 내 Max/Min 계산
    // ...

    // 출력: x=MaxCoC, y=MinCoC
    g_TileOutput[GroupId.xy] = float4(sharedMaxCoC[0], minCoC, 0, 0);
}
```

### Dilate Pass (Compute Shader)

```hlsl
// DOF_TileDilate_CS.hlsl
// DILATE_RADIUS = 2 (최대 2타일 = 16픽셀 거리)

[numthreads(8, 8, 1)]
void mainCS(uint3 DTid)
{
    float maxCoC = centerTile.x;

    // Ring별로 이웃 타일 확인
    for (int ring = 1; ring <= DILATE_RADIUS; ring++)
    {
        for (이웃 타일 순회)
        {
            // 이웃의 CoC가 이 거리까지 도달 가능한지 체크
            if (neighborMaxCoC * CocRadiusToTileScale > ringDistance)
            {
                maxCoC = max(maxCoC, neighborMaxCoC);
            }
        }
    }

    g_TileOutput[DTid.xy] = float4(maxCoC, minCoC, 0, 0);
}
```

### 파일 위치
- **셰이더**:
  - `Mundi/Shaders/PostProcess/DOF_TileFlatten_CS.hlsl`
  - `Mundi/Shaders/PostProcess/DOF_TileDilate_CS.hlsl`
- **C++ Pass**: `Mundi/Source/Runtime/Renderer/PostProcessing/DOFTilePass.cpp`

---

## 3. Blur Pass

### 역할
- Ring-based 2D Gather로 원형 보케(Bokeh) 블러 생성
- Near/Far 분리 처리 (2패스)
- **Premultiplied Alpha 출력**

### 입력
| Slot | Resource | Description |
|------|----------|-------------|
| t0 | DOF[0] | Setup 결과 (RGB + CoC) |
| t1 | Tile SRV | Dilated Max CoC |

### 출력
| Pass | Target | Content |
|------|--------|---------|
| Near | DOF[1] | Near Blurred (Premultiplied Alpha) |
| Far | DOF[2] | Far Blurred (Premultiplied Alpha) |

### Ring-based Gather 알고리즘

```hlsl
// DOF_Blur_PS.hlsl:74-158

// Ring 설정: 0(중심) + 1~5 = 총 121 샘플
const int MAX_RING = 5;

for (int ring = 1; ring <= MAX_RING; ring++)
{
    float ringRadius = pixelRadius * (float(ring) / float(MAX_RING));
    int sampleCount = ring * 8;  // Ring 1=8, Ring 2=16, ...

    for (int s = 0; s < sampleCount; s++)
    {
        float angle = s * (2π / sampleCount);
        float2 offset = float2(cos(angle), sin(angle)) * ringRadius;

        // Scatter-as-Gather: 샘플의 CoC가 현재 위치까지 도달하는지 검사
        float coverage = saturate((sampleRadiusPx - ringRadius + 1.0) / sampleRadiusPx);

        // Area-based weight: 1 / (π × r²)
        // 큰 CoC = 에너지 분산 = 낮은 weight
        float areaWeight = 1.0 / max(PI * r * r, minArea);

        // Layer Processing: Near/Far 분리
        bool isCorrectLayer = (IsFarField == 0) ? (cocSigned < 0) : (cocSigned >= 0);
        if (!isCorrectLayer) continue;

        accColor += sampleColor * weight;
        accCoc += sampleCoc * weight;
        accWeight += weight;
    }
}
```

### Premultiplied Alpha 출력

```hlsl
// DOF_Blur_PS.hlsl:160-184

// 평균 색상
float3 avgColor = accColor / accWeight;

// Opacity: 평균 CoC 기반 (CoC가 클수록 불투명)
float blurredCoc = accCoc / accWeight;
float opacity = saturate(blurredCoc);

// Premultiplied Alpha 출력
output.Color.rgb = avgColor * opacity;  // 색상 × 불투명도
output.Color.a = opacity;                // 불투명도
```

### 파일 위치
- **셰이더**: `Mundi/Shaders/PostProcess/DOF_Blur_PS.hlsl`
- **C++ Pass**: `Mundi/Source/Runtime/Renderer/PostProcessing/DOFBlurPass.cpp`

---

## 4. Scatter Pass

### 역할
- 큰 CoC 픽셀을 Point Sprite로 확장 (보케 효과)
- Gather로 커버하지 못하는 엣지 확장 보완
- Blur 결과 위에 Additive 합성

### 입력
| Slot | Resource | Description |
|------|----------|-------------|
| t0 | DOF[0] | Setup 결과 (RGB + CoC) |

### 출력
| Pass | Target | Blend Mode |
|------|--------|------------|
| Near Scatter | DOF[1] | Premultiplied Alpha Blend |
| Far Scatter | DOF[2] | Premultiplied Alpha Blend |

### Vertex Shader - Point Sprite 생성

```hlsl
// DOF_Scatter_VS.hlsl

// 인스턴스당 4개 정점 (Triangle Strip)
VS_OUTPUT mainVS(uint VertexID, uint InstanceID)
{
    // 픽셀 위치 계산
    uint pixelX = (InstanceID + PixelOffset) % ScreenSize.x;
    uint pixelY = (InstanceID + PixelOffset) / ScreenSize.x;

    // CoC와 색상 읽기
    float4 texData = g_InputTex.Load(int3(pixelX, pixelY, 0));
    float coc = abs(texData.a);

    // Scatter 조건: CoC 임계값 이상 + 해당 레이어
    bool shouldScatter = correctPass && (coc > CocThreshold) && (cocRadiusPx > 3.0);

    if (!shouldScatter)
    {
        // 화면 밖으로 이동 (컬링)
        output.Position = float4(10.0, 10.0, 0.0, 1.0);
        return output;
    }

    // 쿼드 생성 (CoC 크기만큼)
    float2 radiusNDC = cocRadiusPx * TexelSize * 2.0;
    float2 vertexNDC = centerNDC + QuadOffsets[VertexID] * radiusNDC;

    output.Position = float4(vertexNDC, 0.5, 1.0);
    output.CocRadius = cocRadiusPx;
}
```

### Pixel Shader - 원형 보케 렌더링

```hlsl
// DOF_Scatter_PS.hlsl

PS_OUTPUT mainPS(PS_INPUT input)
{
    // 원형 마스크
    float2 uv = input.TexCoord * 2.0 - 1.0;
    float dist = length(uv);

    if (dist > 1.0) discard;  // 원 바깥 버림

    // 부드러운 엣지
    float alpha = 1.0 - smoothstep(0.9, 1.0, dist);

    // Area-based Weight
    float area = PI * input.CocRadius * input.CocRadius;
    float weight = 1.0 / max(area, minArea);

    // Premultiplied Alpha 출력
    output.Color.rgb = input.Color * alpha * weight;
    output.Color.a = alpha * weight;
}
```

### Blend State 설정 (C++)

```cpp
// DOFScatterPass.cpp:117-133

// Premultiplied Alpha Blend State
blendDesc.RenderTarget[0].BlendEnable = TRUE;
blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;              // scatter.rgb
blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;   // blur × (1 - scatter.a)
blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
// Result = Scatter.RGB + Blur.RGB × (1 - Scatter.A)
```

### 파일 위치
- **셰이더**:
  - `Mundi/Shaders/PostProcess/DOF_Scatter_VS.hlsl`
  - `Mundi/Shaders/PostProcess/DOF_Scatter_PS.hlsl`
- **C++ Pass**: `Mundi/Source/Runtime/Renderer/PostProcessing/DOFScatterPass.cpp`

---

## 5. Recombine Pass

### 역할
- Full Resolution Scene Color와 블러 레이어 합성
- 언리얼 스타일 레이어 합성 (Far → Sharp → Near)
- Bilateral Upsampling으로 엣지 보존

### 입력
| Slot | Resource | Description |
|------|----------|-------------|
| t0 | SceneColor | Full Resolution Scene Color |
| t1 | SceneDepth | Full Resolution Depth |
| t2 | DOF[1] | Near Blurred + Scatter |
| t3 | DOF[2] | Far Blurred + Scatter |

### 출력
| Target | Content |
|--------|---------|
| SceneColor | 최종 DOF 합성 결과 |

### 레이어 합성 알고리즘

```hlsl
// DOF_Recombine_PS.hlsl:129-150

// 1. CoC 기반 In-Focus Opacity 계산
float absCoc = abs(CoC);
float focusOpacity = saturate(1.0 - absCoc / maxRecombineCocRadius);
// CoC 작으면 → focusOpacity = 1 (Full Res 사용)
// CoC 크면 → focusOpacity = 0 (Blur 사용)

// 2. Background 합성 (Far blur + Full res blending)
// Far blur의 unpremultiplied color 복원
float3 farUnpremultiplied = (farBlurred.a > 0.001)
    ? farBlurred.rgb / farBlurred.a
    : farBlurred.rgb;

float3 finalColor = farUnpremultiplied * (1.0 - focusOpacity)
                  + sceneColor.rgb * focusOpacity;

// 3. Foreground 합성 (Near blur를 위에 덮음 - Premultiplied)
float nearTranslucency = 1.0 - nearBlurred.a;  // 투명도
finalColor = finalColor * nearTranslucency + nearBlurred.rgb;
```

### 합성 수식 (언리얼 엔진 방식)

```
Layer 1: Background = Far × (1 - focusOpacity) + FullRes × focusOpacity
Layer 2: Final = Background × (1 - nearOpacity) + Near.rgb
```

**Near가 In-Focus를 가리는 원리:**
- Near opacity = 1 이면 → nearTranslucency = 0
- finalColor = Background × 0 + Near.rgb = Near.rgb (뒤가 완전히 가려짐)

### Bilateral Upsampling

```hlsl
// DOF_Recombine_PS.hlsl:68-93

float4 BilateralUpsample(Texture2D blurTex, float2 uv, float centerDepth, float2 lowResTexelSize)
{
    float4 blur = 0;
    float totalWeight = 0;

    for (int y = -1; y <= 1; y++)
    {
        for (int x = -1; x <= 1; x++)
        {
            float2 sampleUV = uv + float2(x, y) * lowResTexelSize;
            float4 s = blurTex.Sample(LinearSampler, sampleUV);

            // 깊이 차이 기반 가중치 (엣지 보존)
            float sampleDepth = LinearizeDepth(depthTex.Sample(PointSampler, sampleUV).r);
            float depthDiff = abs(sampleDepth - centerDepth) / max(centerDepth, 0.1);
            float w = exp(-depthDiff * 2.0);

            blur += s * w;
            totalWeight += w;
        }
    }

    return blur / max(totalWeight, 0.001);
}
```

### 파일 위치
- **셰이더**: `Mundi/Shaders/PostProcess/DOF_Recombine_PS.hlsl`
- **C++ Pass**: `Mundi/Source/Runtime/Renderer/PostProcessing/DOFRecombinePass.cpp`

---

## Premultiplied Alpha 설명

### 왜 Premultiplied Alpha를 사용하는가?

1. **Gather 누적의 수학적 정확성**
   - 여러 샘플을 가중 합산할 때 에너지 보존
   - `RGB = Σ(color × weight)`, `A = Σ(weight)`

2. **효율적인 블렌딩**
   - 단일 연산으로 합성: `dst × (1-α) + src.rgb`
   - 추가 곱셈 불필요

3. **레이어 합성 일관성**
   - Scatter와 Blur 결과를 자연스럽게 합성
   - Near가 Far/Sharp를 올바르게 가림

### Premultiplied vs Non-Premultiplied

| 방식 | RGB | Alpha | 블렌딩 |
|------|-----|-------|--------|
| **Premultiplied** | color × α | α | dst×(1-α) + src.rgb |
| **Non-Premultiplied** | color | α | dst×(1-α) + src.rgb×α |

### DOF 시스템에서의 적용

```
[Blur Pass]
  RGB = avgColor × opacity
  A = opacity

[Scatter Pass]
  RGB = color × intensity × weight
  A = intensity × weight

[Recombine Pass]
  // Near Premultiplied 블렌딩
  finalColor = finalColor × (1 - nearBlurred.a) + nearBlurred.rgb
```

---

## 상수 버퍼 구조

### DOFSetupCB / DOFRecombineCB (b2)

```cpp
cbuffer DOFSetupCB : register(b2)
{
    float FocalDistance;           // 초점 거리 (m)
    float FocalRegion;             // 완전 선명 영역 (m)
    float NearTransitionRegion;    // 근경 전환 영역 (m)
    float FarTransitionRegion;     // 원경 전환 영역 (m)

    float MaxNearBlurSize;         // 근경 최대 블러 (pixels)
    float MaxFarBlurSize;          // 원경 최대 블러 (pixels)
    float NearClip;                // Near Clip Plane
    float FarClip;                 // Far Clip Plane

    int IsOrthographic;            // 직교 투영 여부
    float _Pad0;
    float2 ViewRectMinUV;          // ViewRect 시작 UV

    float2 ViewRectMaxUV;          // ViewRect 끝 UV
    float2 _Pad1;
}
```

### ViewportConstants (b10)

```cpp
cbuffer ViewportConstants : register(b10)
{
    float4 ViewportRect;  // x, y, width, height
    float4 ScreenSize;    // width, height, 1/width, 1/height
}
```

---

## 파라미터 가이드

| 파라미터 | 범위 | 권장값 | 설명 |
|----------|------|--------|------|
| FocalDistance | 0.1 ~ 1000 m | 5 ~ 50 m | 초점 거리 |
| FocalRegion | 0 ~ 100 m | 1 ~ 10 m | 완전 선명 영역 |
| NearTransitionRegion | 0.1 ~ 50 m | 1 ~ 5 m | 근경 전환 부드러움 |
| FarTransitionRegion | 0.1 ~ 100 m | 5 ~ 20 m | 원경 전환 부드러움 |
| MaxNearBlurSize | 1 ~ 64 px | 16 ~ 32 px | 근경 최대 블러 |
| MaxFarBlurSize | 1 ~ 64 px | 16 ~ 32 px | 원경 최대 블러 |

---

## 파일 구조

```
Mundi/
├── Shaders/PostProcess/
│   ├── DOF_Common.hlsli          # 공통 함수 (CoC 계산, 다운샘플링)
│   ├── DOF_Setup_PS.hlsl         # Setup Pass 픽셀 셰이더
│   ├── DOF_TileFlatten_CS.hlsl   # Tile Flatten 컴퓨트 셰이더
│   ├── DOF_TileDilate_CS.hlsl    # Tile Dilate 컴퓨트 셰이더
│   ├── DOF_Blur_PS.hlsl          # Blur Pass 픽셀 셰이더
│   ├── DOF_Scatter_VS.hlsl       # Scatter Pass 버텍스 셰이더
│   ├── DOF_Scatter_PS.hlsl       # Scatter Pass 픽셀 셰이더
│   └── DOF_Recombine_PS.hlsl     # Recombine Pass 픽셀 셰이더
│
└── Source/Runtime/Renderer/PostProcessing/
    ├── DOFSetupPass.cpp/.h       # Setup Pass C++ 구현
    ├── DOFTilePass.cpp/.h        # Tile Pass C++ 구현
    ├── DOFBlurPass.cpp/.h        # Blur Pass C++ 구현
    ├── DOFScatterPass.cpp/.h     # Scatter Pass C++ 구현
    └── DOFRecombinePass.cpp/.h   # Recombine Pass C++ 구현
```

---

## 성능 고려사항

### 해상도
- Setup/Blur/Scatter는 **Half Resolution** (1/4 픽셀 수)
- Recombine만 Full Resolution

### 샘플 수
- Blur: 121 샘플 (Ring 0~5)
- Scatter: 픽셀당 4 정점 (Draw Instanced)

### 최적화 기법
1. **Tile-based CoC**: 검색 범위 제한
2. **Early Out**: 타일 MaxCoC < 0.01이면 스킵
3. **Layer Separation**: Near/Far 독립 처리로 오버드로우 감소
4. **Bilateral Upsampling**: 엣지 보존으로 낮은 해상도에서도 품질 유지

---

## 알려진 제한사항

1. **투명 오브젝트**: 현재 Depth 기반으로 처리 못함
2. **매우 큰 CoC**: Tile Dilate 범위 초과 시 아티팩트
3. **성능**: Scatter Pass의 픽셀당 쿼드 렌더링이 느릴 수 있음

---

## 참고 자료

- [Unreal Engine 5 - Cinematic DOF](https://docs.unrealengine.com/5.0/en-US/depth-of-field-in-unreal-engine/)
- [SIGGRAPH 2018 - A Life of a Bokeh](https://advances.realtimerendering.com/s2018/)
- DOFRecombine.usf (언리얼 엔진 소스)
- DOFGatherAccumulator.ush (언리얼 엔진 소스)

---

*Last Updated: 2025-12-01*
*Author: Claude Code*
