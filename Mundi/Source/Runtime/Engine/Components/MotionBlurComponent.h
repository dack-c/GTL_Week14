#pragma once

#include "Object.h"

#include "SceneComponent.h"
#include "UMotionBlurComponent.generated.h"


UCLASS(DisplayName = "Depth Of Field", Description = "피사계 심도")
class UMotionBlurComponent : public USceneComponent
{
public:

    GENERATED_REFLECTION_BODY()

public:

    UMotionBlurComponent() = default;
    ~UMotionBlurComponent() override = default;


    // Rendering
    void RenderHeightFog(URenderer* Renderer);

    void OnRegister(UWorld* InWorld) override;
    // Serialize
    void Serialize(const bool bInIsLoading, JSON& InOutHandle) override;

    // ───── 복사 관련 ────────────────────────────
    void DuplicateSubObjects() override;

public:

    UPROPERTY(EditAnywhere, Category = "MotionBlur", Range = "0.00, 2.0")
    float MaxVelocity = 0.2f;

    UPROPERTY(EditAnywhere, Category = "MotionBlur", Range = "1, 32")
    int SampleCount = 8;

    UPROPERTY(EditAnywhere, Category = "MotionBlur", Range = "1.0, 30.0")
    float GaussianBlurWeight = 1;

private:

};
