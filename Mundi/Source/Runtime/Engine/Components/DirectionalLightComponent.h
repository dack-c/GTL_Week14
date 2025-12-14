#pragma once

#include "LightComponent.h"
#include "LightManager.h"
#include "UDirectionalLightComponent.generated.h"

// 방향성 라이트 (태양광 같은 평행광)
UCLASS(DisplayName="방향성 라이트 컴포넌트", Description="평행광 조명 컴포넌트입니다")
class UDirectionalLightComponent : public ULightComponent
{
public:

	GENERATED_REFLECTION_BODY()

		UDirectionalLightComponent();
	virtual ~UDirectionalLightComponent() override;

public:
	void GetShadowRenderRequests(FSceneView* View, TArray<FShadowRenderRequest>& OutRequests) override;

	// 월드 회전을 반영한 라이트 방향 반환 (Transform의 Forward 벡터)
	FVector GetLightDirection() const;

	// Light Info
	FDirectionalLightInfo GetLightInfo() const;

	// Virtual Interface
	void OnRegister(UWorld* InWorld) override;
	void OnUnregister() override;
	virtual void UpdateLightData() override;
	void OnTransformUpdated() override;

	// Serialization & Duplication
	void Serialize(const bool bInIsLoading, JSON& InOutHandle) override;
	virtual void DuplicateSubObjects() override;

	// Update Gizmo to match light properties
	void UpdateDirectionGizmo();

	bool IsOverrideCameraLightPerspective() { return bOverrideCameraLightPerspective; }

protected:
	// Direction Gizmo (shows light direction)
	class UGizmoArrowComponent* DirectionGizmo = nullptr;

	UPROPERTY(EditAnywhere, Category="ShadowMap")
	ID3D11ShaderResourceView* ShadowMapSRV = nullptr;

private:
	UPROPERTY(EditAnywhere, Category="ShadowMap")
	bool bCascaded = true;

	UPROPERTY(EditAnywhere, Category="ShadowMap", Range="1, 8")
	int CascadedCount = 4;

	UPROPERTY(EditAnywhere, Category="ShadowMap", Range="0, 1")
	float CascadedLinearBlendingValue = 0.5f;

	UPROPERTY(EditAnywhere, Category = "ShadowMap", Range = "0.1, 0.95", DisplayName = "캐스케이드 분할 람다", Tooltip="첫 N-1 캐스케이드와 마지막 캐스케이드의 분할 비율을 결정합니다.")
	float CascadeSplitLambda = 0.5f;

	UPROPERTY(EditAnywhere, Category="ShadowMap", Range="0, 0.5")
	float CascadedOverlapValue = 0.2f;

	bool bOverrideCameraLightPerspective = false;
	TArray<float> CascadedSliceDepth;

	//로그용
	UPROPERTY(EditAnywhere, Category="ShadowMap", Range="0, 1.0")
	float CascadedAreaColorDebugValue = 0;

	UPROPERTY(EditAnywhere, Category="ShadowMap", Range="-1, 8")
	int CascadedAreaShadowDebugValue = -1;

	UPROPERTY(EditAnywhere, Category="ShadowMap", DisplayName="최대 그림자 거리", Tooltip="그림자가 렌더링될 최대 거리입니다. 0 이하일 경우 카메라 FarClip을 따릅니다.")
	float MaxShadowDistance = 5000.0f; // 0 이하일 경우 카메라 FarClip 사용
};
