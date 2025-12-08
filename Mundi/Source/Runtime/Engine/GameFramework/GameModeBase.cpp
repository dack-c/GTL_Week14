#include "pch.h"
#include "GameModeBase.h"
#include "PlayerController.h"
#include "Pawn.h"
#include "World.h"
#include "CameraComponent.h"
#include "SpringArmComponent.h"
#include "PlayerCameraManager.h"
#include "Character.h"
#include "PlayerStart.h"
#include "Source/Runtime/Core/Misc/PathUtils.h"

AGameModeBase::AGameModeBase()
{
	//DefaultPawnClass = APawn::StaticClass();
	DefaultPawnClass = ACharacter::StaticClass();
	PlayerControllerClass = APlayerController::StaticClass();
}

AGameModeBase::~AGameModeBase()
{
}

void AGameModeBase::StartPlay()
{
	//TODO 
	//GameState 세팅 
	Login();
	PostLogin(PlayerController);
}

APlayerController* AGameModeBase::Login()
{
	if (PlayerControllerClass)
	{
		PlayerController = Cast<APlayerController>(GWorld->SpawnActor(PlayerControllerClass, FTransform())); 
	}
	else
	{
		PlayerController = GWorld->SpawnActor<APlayerController>(FTransform());
	}

	return PlayerController;
}

void AGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	AActor* StartSpot = FindPlayerStart(NewPlayer);
	APawn* NewPawn = NewPlayer->GetPawn();

    if (!NewPawn && NewPlayer)
    {
        {
            FWideString PrefabPath = UTF8ToWide(GDataDir) + L"/Prefabs/GameJem.prefab";
            if (AActor* PrefabActor = GWorld->SpawnPrefabActor(PrefabPath))
            {
                if (APawn* PrefabPawn = Cast<APawn>(PrefabActor))
                {
                    NewPawn = PrefabPawn;
                }
            }
        }

        if (!NewPawn)
        {
            NewPawn = SpawnDefaultPawnFor(NewPlayer, StartSpot);
        }

        if (NewPawn && StartSpot)
        {
            NewPawn->SetActorLocation(StartSpot->GetActorLocation());
            NewPawn->SetActorRotation(StartSpot->GetActorRotation());
        }

        if (NewPawn)
        {
            NewPlayer->Possess(NewPawn);
        }
    }

	if (NewPawn && !NewPawn->GetComponent(USpringArmComponent::StaticClass()))
	{
		USpringArmComponent* SpringArm = nullptr;
		if (UActorComponent* SpringArmComp = NewPawn->AddNewComponent(USpringArmComponent::StaticClass(), NewPawn->GetRootComponent()))
		{
			SpringArm = Cast<USpringArmComponent>(SpringArmComp);
			SpringArm->SetRelativeLocation(FVector(0, 0, 2.0f));
			SpringArm->SetTargetArmLength(10.0f);
			SpringArm->SetDoCollisionTest(true);
			SpringArm->SetUsePawnControlRotation(true);
		}

		if (SpringArm)
		{
			if (UActorComponent* CameraComp = NewPawn->AddNewComponent(UCameraComponent::StaticClass(), SpringArm))
			{
				auto* Camera = Cast<UCameraComponent>(CameraComp);
				Camera->SetRelativeLocation(FVector(0, 0, 0));
				Camera->SetRelativeRotationEuler(FVector(0, 0, 0));
			}
		}
	}

	if (auto* PCM = GWorld->GetPlayerCameraManager())
	{
		if (auto* Camera = Cast<UCameraComponent>(NewPawn->GetComponent(UCameraComponent::StaticClass())))
		{
			PCM->SetViewCamera(Camera);
		}
	}

	if (NewPlayer)
	{
		NewPlayer->Possess(NewPlayer->GetPawn());
	}
}

APawn* AGameModeBase::SpawnDefaultPawnFor(AController* NewPlayer, AActor* StartSpot)
{
	FVector SpawnLoc = FVector::Zero();
	FQuat SpawnRot = FQuat::Identity();

	if (StartSpot)
	{
		SpawnLoc = StartSpot->GetActorLocation();
		SpawnRot = StartSpot->GetActorRotation();
	}

	if (DefaultPawnClass)
	{
		return Cast<APawn>(GetWorld()->SpawnActor(DefaultPawnClass, FTransform(SpawnLoc, SpawnRot, FVector(1, 1, 1))));
	}

	return nullptr;
}

AActor* AGameModeBase::FindPlayerStart(AController* Player)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	TArray<APlayerStart*> AllPlayerStarts;
	APlayerStart* PIEPlayerStart = nullptr;

	const TArray<AActor*>& AllActors = World->GetLevel()->GetActors();
	for (AActor* Actor : AllActors)
	{
		if (APlayerStart* PS = Cast<APlayerStart>(Actor))
		{
			if (PS->IsPIEPlayerStart())
			{
				PIEPlayerStart = PS;
				break;
			}

			AllPlayerStarts.push_back(PS);
		}
	}

	if (PIEPlayerStart)
	{
		return PIEPlayerStart;
	}

	if (AllPlayerStarts.size() > 0)
	{
		int32 RandomIndex = rand() % static_cast<int32>(AllPlayerStarts.size());
		return AllPlayerStarts[RandomIndex];
	}

	return nullptr;
}
  
