#include "pch.h"
#include "AnimNotify_PlaySound.h"
#include "SkeletalMeshComponent.h"
#include "AnimSequenceBase.h"
#include "Source/Runtime/Engine/GameFramework/FAudioDevice.h"
#include "Source/Runtime/Engine/Scripting/LuaManager.h"

IMPLEMENT_CLASS(UAnimNotify_PlaySound)
void UAnimNotify_PlaySound::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (Sound && MeshComp)
	{
		// 게임 상태 체크: Playing 상태일 때만 효과음 재생
		FString GameState = FLuaManager::GetGlobalString("GlobalConfig.GameState");

		if (GameState != "Playing")
		{
			return;  // Death, Clear, Init 상태에서는 효과음 재생 안 함
		}

		// Sound 재생
		AActor* Owner = MeshComp->GetOwner();
		FVector SoundPos = MeshComp->GetWorldLocation();
		FAudioDevice::PlaySoundAtLocationOneShot(Sound, SoundPos);
	}
}
