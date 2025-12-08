#pragma once
#include "PawnMovementComponent.h"
#include "Collision.h"
#include "UCharacterMovementComponent.generated.h"

class ACharacter;
struct FHitResult;
struct FPhysScene;
 
class UCharacterMovementComponent : public UPawnMovementComponent
{	
public:
	GENERATED_REFLECTION_BODY()

	UCharacterMovementComponent();
	virtual ~UCharacterMovementComponent() override;

	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaSeconds) override;

	// 캐릭터 전용 설정 값
	float MaxWalkSpeed;
	float MaxAcceleration;
	float JumpZVelocity; 

	float BrackingDeceleration; // 입력이 없을 때 감속도
	float GroundFriction; //바닥 마찰 계수 

	bool bUseGravity = true;
	bool bUseInput = true;

	//TODO
	//float MaxWalkSpeedCrouched = 6.0f;
	//float MaxSwimSpeed;
	//float MaxFlySpeed;

	UFUNCTION(LuaBind, DisplayName = "ResetVelocity")
	void ResetVelocity()
	{
		Velocity = FVector::Zero();
		Acceleration = FVector::Zero();
	}

	// 상태 제어 
	void DoJump();
	void StopJump();
	bool TryStartSliding();
	bool IsFalling() const { return bIsFalling; }
	bool IsSliding() const { return bIsSliding; }
	bool IsJumping() const { return bIsJumping; }
	bool NeedRolling() const { return bNeedRolling; }
	bool IsOnGround() const { return !bIsFalling; }
	const FHitResult& GetCurrentFloorResult() const { return CurrentFloor; }

	UFUNCTION(LuaBind, DisplayName = "SetUseGravity")
	void SetUseGravity(bool bEnable) { bUseGravity = bEnable; }

	UFUNCTION(LuaBind, DisplayName = "IsUsingGravity")
	bool IsUsingGravity() const { return bUseGravity; }
	
	void SetUseInput(bool bEnable)
	{ 
		if (bEnable == bUseInput)
		{
			return;
		}

		/*if (!bEnable)
		{
			LastVelocityBeforIgnoreInput = Velocity;
			Velocity = FVector::Zero();
		}
		else
		{
			Velocity = LastVelocityBeforIgnoreInput;
			LastVelocityBeforIgnoreInput = FVector::Zero();
		}*/
		bUseInput = bEnable;
	}
	bool IsUsingInput() const { return bUseInput; }

protected:
	void PhysSliding(float DeltaSecond);
	void PhysWalking(float DeltaSecond);
	void PhysFalling(float DeltaSecond);

	void CalcVelocity(const FVector& Input, float DeltaSecond, float Friction, float BrackingDecel);

	/**
	 * @brief Sweep 검사를 통해 안전한 이동 수행
	 * @param Delta 이동할 거리
	 * @param OutHit 충돌 결과
	 * @return 실제 이동 완료 여부 (충돌 시 false)
	 */
	bool SafeMoveUpdatedComponent(const FVector& Delta, FHitResult& OutHit);

	/**
	 * @brief 충돌 후 슬라이딩 처리
	 * @param Delta 원래 이동 벡터
	 * @param Hit 충돌 정보
	 * @return 슬라이딩 후 남은 이동 벡터
	 */
	FVector SlideAlongSurface(const FVector& Delta, const FHitResult& Hit);

	/**
	 * @brief 바닥 검사 (아래로 Sweep)
	 * @param OutHit 바닥 충돌 결과
	 * @return 바닥에 서있는지 여부
	 */
	bool CheckFloor(FHitResult& OutHit);

	/** 캡슐 컴포넌트 크기 가져오기 */
	void GetCapsuleSize(float& OutRadius, float& OutHalfHeight) const;

	FVector GetSnapDownStart() const;

	/** PhysScene 가져오기 */
	FPhysScene* GetPhysScene() const;

	/**
	 * @brief 캐릭터의 슬라이딩 상태를 설정합니다.
	 * 상태 변경에 따라 파티클 효과를 활성화/비활성화합니다.
	 * @param bNewIsSliding 새로운 슬라이딩 상태
	 */
	void SetSliding(bool bNewIsSliding);

	/**
	 * @brief 현재 위치에서 관통(Penetration) 해결
	 * 반복적으로 MTD를 계산하여 겹친 상태에서 탈출
	 * @return 탈출에 성공하면 true, 최대 반복 후에도 실패하면 false
	 */
	bool ResolveOverlaps();

public:
	UPROPERTY(EditAnywhere, Category = "Move")
	float SlidingSpeed = 20.0f;
	
	UPROPERTY(EditAnywhere, Category = "Move")
	float SlidingRotateSpeed = 10.0f;
	
	UPROPERTY(EditAnywhere, Category = "Move")
	float MinSlidingSpeed = 0.2f;

	UPROPERTY(EditAnywhere, Category = "Move")
	float MinFloorNormalZ = 0.7f;	// 평지로 인식하는 최소 normal Z

	UPROPERTY(EditAnywhere, Category = "Move")
	float NeedRollingAirTime = 1.0f;	// 해당 시간 동안 체공 시 구르기로 착지

	UPROPERTY(EditAnywhere, Category = "Capsule")
	FVector CapsuleOffset = FVector::Zero(); // 캡슐 콜라이더 오프셋

	//UPROPERTY(EditAnywhere, Category = "Move")
	float SlideFloorMaxNormalZ = 0.96f;	// 미끄러 질 수 있는 바닥의 최대 normal Z

	const float LateralSpeed = 10.0f; // 슬라이딩 중 좌우 이동 속도

protected:
	ACharacter* CharacterOwner = nullptr;
	bool bIsJumping = false;
	bool bIsFalling = false;
	bool bIsSliding = false;
	bool bNeedRolling = false;
	float AirTime = 0.0f;
	FHitResult CurrentFloor;

	const float GLOBAL_GRAVITY_Z = -9.8f;
	const float GravityScale = 1.0f;

	const float SkinWidth = 0.00125;  // 통일된 SkinWidth

	FVector LastVelocityBeforIgnoreInput = FVector::Zero();
};
