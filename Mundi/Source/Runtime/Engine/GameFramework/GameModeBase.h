#pragma once
#include "Actor.h"
#include "GameModeBase.generated.h"

class APawn;
class APlayerController;


class AGameModeBase : public AActor
{
public:
	GENERATED_REFLECTION_BODY()

	AGameModeBase();
	~AGameModeBase() override;
	
	APawn* DefaultPawnClass;
	APlayerController* PlayerControllerClass;

	// 게임 시작 시 호출
	virtual void StartPlay();

	// 플레이어 리스폰할 때 사용
	virtual void RestartPlayer(APlayerController* NewPlayer); 		 
};