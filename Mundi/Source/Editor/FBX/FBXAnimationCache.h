#pragma once
#include "String.h"

class UAnimSequence;

class FBXAnimationCache
{
public:
	// 캐시 디렉토리에서 애니메이션 로드 시도
	static bool TryLoadAnimationsFromCache(const FString& NormalizedPath, TArray<UAnimSequence*>& OutAnimations);

	// 단일 애니메이션을 캐시 파일에 저장
	static bool SaveAnimationToCache(UAnimSequence* Animation, const FString& CachePath);

	// 캐시 파일에서 단일 애니메이션 로드
	static UAnimSequence* LoadAnimationFromCache(const FString& CachePath);
};
