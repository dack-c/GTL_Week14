#pragma once
#include "PrimitiveComponent.h"
#include "ConstantBufferType.h"
#include "USkySphereComponent.generated.h"

class UShader;
class UStaticMesh;
struct FMeshBatchElement;

UCLASS(DisplayName="스카이 스피어", Description="스카이 렌더링 컴포넌트")
class USkySphereComponent : public UPrimitiveComponent
{
	GENERATED_REFLECTION_BODY()

public:
	// Sky 파라미터 (에디터 노출)
	UPROPERTY(EditAnywhere, Category="Sky|Colors", DisplayName="천정 색상")
	FLinearColor ZenithColor = FLinearColor(0.0343f, 0.1236f, 0.4f, 1.0f);

	UPROPERTY(EditAnywhere, Category="Sky|Colors", DisplayName="수평선 색상")
	FLinearColor HorizonColor = FLinearColor(0.6471f, 0.8235f, 0.9451f, 1.0f);

	UPROPERTY(EditAnywhere, Category="Sky|Colors", DisplayName="지면 색상")
	FLinearColor GroundColor = FLinearColor(0.3f, 0.25f, 0.2f, 1.0f);

	UPROPERTY(EditAnywhere, Category="Sky|Sun", DisplayName="태양 방향")
	FVector SunDirection = FVector(0.0f, 0.5f, 0.866f);

	UPROPERTY(EditAnywhere, Category="Sky|Sun", DisplayName="태양 크기")
	float SunDiskSize = 0.001f;

	UPROPERTY(EditAnywhere, Category="Sky|Sun", DisplayName="태양 색상")
	FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 5.0f);

	UPROPERTY(EditAnywhere, Category="Sky|Atmosphere", DisplayName="수평선 감쇠")
	float HorizonFalloff = 3.0f;

	UPROPERTY(EditAnywhere, Category="Sky|Atmosphere", DisplayName="전체 밝기")
	float OverallBrightness = 1.0f;

	UPROPERTY(EditAnywhere, Category="Sky|Sun", DisplayName="태양 방향 자동 업데이트")
	bool bAutoUpdateSunDirection = true;

	USkySphereComponent();
	~USkySphereComponent() override;

	// UActorComponent Interface
	void BeginPlay() override;
	void TickComponent(float DeltaSeconds) override;

	// UPrimitiveComponent Interface
	FAABB GetWorldAABB() const override;
	void CollectMeshBatches(TArray<FMeshBatchElement>& OutMeshBatchElements, const FSceneView* View) override;

protected:
	FSkyConstantBuffer SkyParams;
	UStaticMesh* SphereMesh = nullptr;
	UShader* SkyShader = nullptr;
	float SphereRadius = 10000.0f;  // 퓨쳐엔진과 동일한 크기

	void SyncToConstantBuffer();
	void EnsureResourcesLoaded();
	void CreateSphereMesh();
	void LoadShader();
	void UpdateSunDirectionFromLight();
};
