#include "pch.h"
#include "Character.h"
#include "CapsuleComponent.h"
#include "SkeletalMeshComponent.h"
#include "CharacterMovementComponent.h"
#include "Source/Runtime/Engine/Components/ParticleSystemComponent.h"
#include "ObjectMacros.h" 
#include "Source/Runtime/Engine/Collision/Collision.h" 
#include "TriggerComponent.h"

ACharacter::ACharacter()
{
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>("CapsuleComponent");
	//CapsuleComponent->SetSize();

	SetRootComponent(CapsuleComponent);

	if (SkeletalMeshComp)
	{
		SkeletalMeshComp->SetupAttachment(CapsuleComponent);

		//SkeletalMeshComp->SetRelativeLocation(FVector());
		//SkeletalMeshComp->SetRelativeScale(FVector());
	}
	 
	CharacterMovement = CreateDefaultSubobject<UCharacterMovementComponent>("CharacterMovement");
	if (CharacterMovement)
	{
		CharacterMovement->SetUpdatedComponent(CapsuleComponent);
	} 
}

ACharacter::~ACharacter()
{

}

void ACharacter::Tick(float DeltaSecond)
{
	Super::Tick(DeltaSecond);
    UTriggerComponent::CharacterPos = RootComponent->GetWorldLocation();
}

void ACharacter::BeginPlay()
{
    Super::BeginPlay();

	if (SlidingParticleComponent)
	{
		SlidingParticleComponent->bSuppressSpawning = true;
	}
}

void ACharacter::Serialize(const bool bInIsLoading, JSON& InOutHandle)
{
    Super::Serialize(bInIsLoading, InOutHandle);

    if (bInIsLoading)
    {
        // Rebind important component pointers after load (prefab/scene)
        CapsuleComponent = nullptr;
		SkeletalMeshComp = nullptr;
        CharacterMovement = nullptr;
		SlidingParticleComponent = nullptr;
		LandingParticleComponent = nullptr;

        for (UActorComponent* Comp : GetOwnedComponents())
        {
            if (auto* Cap = Cast<UCapsuleComponent>(Comp))
            {
                CapsuleComponent = Cap;
            }
			else if (auto* Skel = Cast<USkeletalMeshComponent>(Comp))
			{
				SkeletalMeshComp = Skel;
			}
            else if (auto* Move = Cast<UCharacterMovementComponent>(Comp))
            {
                CharacterMovement = Move;
            }
			else if (auto* Particle = Cast<UParticleSystemComponent>(Comp))
			{
				// TODO: Find a better way to distinguish particle components
				if (Comp->GetName() == "SlidingParticle")
				{
					SlidingParticleComponent = Particle;
				}
				else if (Comp->GetName() == "LandingParticle")
				{
					LandingParticleComponent = Particle;
				}
			}
        }

        if (CharacterMovement)
        {
            USceneComponent* Updated = CapsuleComponent ? reinterpret_cast<USceneComponent*>(CapsuleComponent)
                                                        : GetRootComponent();
            CharacterMovement->SetUpdatedComponent(Updated);
        }
    }
}

void ACharacter::DuplicateSubObjects()
{ 
    Super::DuplicateSubObjects();
     
    CapsuleComponent = nullptr;
	SkeletalMeshComp = nullptr;
    CharacterMovement = nullptr;
	SlidingParticleComponent = nullptr;
	LandingParticleComponent = nullptr;

    for (UActorComponent* Comp : GetOwnedComponents())
    {
        if (auto* Cap = Cast<UCapsuleComponent>(Comp))
        {
            CapsuleComponent = Cap;
        }
		else if (auto* Skel = Cast<USkeletalMeshComponent>(Comp))
		{
			SkeletalMeshComp = Skel;
		}
        else if (auto* Move = Cast<UCharacterMovementComponent>(Comp))
        {
            CharacterMovement = Move;
        }
		else if (auto* Particle = Cast<UParticleSystemComponent>(Comp))
		{
			if (Comp->GetName() == "SlidingParticle")
			{
				SlidingParticleComponent = Particle;
			}
			else if (Comp->GetName() == "LandingParticle")
			{
				LandingParticleComponent = Particle;
			}
		}
    }

    // Ensure movement component tracks the correct updated component
    if (CharacterMovement)
    {
        USceneComponent* Updated = CapsuleComponent ? reinterpret_cast<USceneComponent*>(CapsuleComponent)
                                                    : GetRootComponent();
        CharacterMovement->SetUpdatedComponent(Updated);
    }
}

void ACharacter::Jump()
{
	if (CharacterMovement)
	{
		CharacterMovement->DoJump();
	}
}

void ACharacter::StopJumping()
{
	if (CharacterMovement)
	{
		// 점프 scale을 조절할 때 사용,
		// 지금은 비어있음
		CharacterMovement->StopJump(); 
	}
}

void ACharacter::TryStartSliding()
{
	if (CharacterMovement)
	{
		CharacterMovement->TryStartSliding();
	}
}

float ACharacter::GetCurrentGroundSlope() const
{
	if (CharacterMovement && CharacterMovement->IsOnGround())
	{
		const FHitResult& FloorResult = CharacterMovement->GetCurrentFloorResult();
		if (FloorResult.bBlockingHit)
		{
			// 바닥의 법선 벡터
			const FVector& FloorNormal = FloorResult.ImpactNormal;

			// 월드 Up 벡터
			const FVector UpVector(0.f, 0.f, 1.f);

			// 두 벡터 사이의 각도를 계산 (라디안 단위)
            float DotResult = FVector::Dot(FloorNormal, UpVector);
            DotResult = FMath::Clamp(DotResult, -1.0f, 1.0f);
 
			float AngleRadians = acosf(DotResult);

			// 각도를 디그리로 변환
			return RadiansToDegrees(AngleRadians);
		}
	}

	return 0.f;
}
