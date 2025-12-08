#pragma once
#include "Actor.h"
#include "APlayerStart.generated.h"

class UBillboardComponent;

/**
 * @brief 플레이어 시작 위치 마커
 * @details 게임 시작 시 플레이어가 스폰될 위치를 지정
 *
 * UE Reference: Engine/Source/Runtime/Engine/Classes/GameFramework/PlayerStart.h
 */
UCLASS(DisplayName="플레이어 시작 위치", Description="플레이어 스폰 위치")
class APlayerStart : public AActor
{
	GENERATED_REFLECTION_BODY()

public:
	APlayerStart();
	~APlayerStart() override = default;

	FName GetPlayerStartTag() const { return PlayerStartTag; }
	void SetPlayerStartTag(FName NewTag) { PlayerStartTag = NewTag; }

	bool IsPIEPlayerStart() const { return bIsPIEPlayerStart; }
	void SetPIEPlayerStart(bool bInIsPIE) { bIsPIEPlayerStart = bInIsPIE; }

protected:
	UPROPERTY(EditAnywhere, Category="PlayerStart")
	FName PlayerStartTag;

	bool bIsPIEPlayerStart;
	UBillboardComponent* EditorIcon;
};
