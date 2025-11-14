#include "pch.h"
#include "AnimNotify_PlaySound.h"
#include "SkeletalMeshComponent.h"
#include "AnimSequenceBase.h" 
#include "Source/Runtime/Engine/GameFramework/FAudioDevice.h"

void UAnimNotify_PlaySound::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	UE_LOG("PlaySound_Notify");
	if (Sound && MeshComp)
	{
		// Sound 재생 
		AActor* Owner = MeshComp->GetOwner();

		FVector SoundPos = MeshComp->GetWorldLocation();

		FAudioDevice::PlaySoundAtLocationOneShot(Sound, SoundPos);
	}
}
