#include "pch.h"
#include "FBXAnimationCache.h"
#include "Source/Runtime/Engine/Animation/AnimSequence.h"
#include "Source/Runtime/Engine/Animation/AnimDateModel.h"
#include "ObjectFactory.h"
#include "WindowsBinReader.h"
#include "WindowsBinWriter.h"
#include "PathUtils.h"
#include <filesystem>

bool FBXAnimationCache::TryLoadAnimationsFromCache(const FString& NormalizedPath, TArray<UAnimSequence*>& OutAnimations)
{
#ifdef USE_OBJ_CACHE
	OutAnimations.Empty();

	FString CacheBasePath = ConvertDataPathToCachePath(NormalizedPath);
	FString AnimCacheDir = CacheBasePath + "_Anims";

	if (!std::filesystem::exists(AnimCacheDir) || !std::filesystem::is_directory(AnimCacheDir))
	{
		return false;
	}

	bool bLoadedAny = false;
	for (const auto& Entry : std::filesystem::directory_iterator(AnimCacheDir))
	{
		if (!Entry.is_regular_file())
		{
			continue;
		}

		if (Entry.path().extension() != ".bin")
		{
			continue;
		}

		FString CachePath = NormalizePath(Entry.path().string());
		if (UAnimSequence* CachedAnim = LoadAnimationFromCache(CachePath))
		{
			FString AnimStackName = CachedAnim->ObjectName.ToString();
			FString AnimKey = NormalizedPath + "_" + AnimStackName;

			if (!RESOURCE.Add<UAnimSequence>(AnimKey, CachedAnim))
			{
				UE_LOG("Animation cache already registered: %s", AnimKey.c_str());
			}

			OutAnimations.Add(CachedAnim);
			bLoadedAny = true;
		}
	}

	if (!bLoadedAny)
	{
		UE_LOG("Animation cache directory '%s' did not contain valid .anim.bin files", AnimCacheDir.c_str());
	}

	return bLoadedAny;
#else
	(void)NormalizedPath;
	(void)OutAnimations;
	return false;
#endif
}

bool FBXAnimationCache::SaveAnimationToCache(UAnimSequence* Animation, const FString& CachePath)
{
	if (!Animation)
	{
		return false;
	}

	try
	{
		FWindowsBinWriter Writer(CachePath);

		UAnimDataModel* DataModel = Animation->GetDataModel();
		if (!DataModel)
		{
			return false;
		}

		// 애니메이션 이름 먼저 쓰기
		FString AnimName = Animation->ObjectName.ToString();
		Serialization::WriteString(Writer, AnimName);

		// 메타데이터 쓰기
		float PlayLength = DataModel->GetPlayLength();
		int32 FrameRate = DataModel->GetFrameRate();
		int32 NumberOfFrames = DataModel->GetNumberOfFrames();
		int32 NumberOfKeys = DataModel->GetNumberOfKeys();

		Writer << PlayLength;
		Writer << FrameRate;
		Writer << NumberOfFrames;
		Writer << NumberOfKeys;

		// 호환성 검사를 위한 본 이름 쓰기
		const TArray<FName>& BoneNames = Animation->GetBoneNames();
		uint32 NumBoneNames = (uint32)BoneNames.Num();
		Writer << NumBoneNames;
		for (const FName& BoneName : BoneNames)
		{
			FString BoneNameStr = BoneName.ToString();
			Serialization::WriteString(Writer, BoneNameStr);
		}

		// 본 트랙 쓰기
		TArray<FBoneAnimationTrack>& Tracks = DataModel->GetBoneAnimationTracks();
		uint32 NumTracks = (uint32)Tracks.Num();
		Writer << NumTracks;

		for (FBoneAnimationTrack& Track : Tracks)
		{
			// 본 이름 쓰기
			FString BoneName = Track.Name.ToString();
			Serialization::WriteString(Writer, BoneName);

			// 위치 키 쓰기
			Serialization::WriteArray(Writer, Track.InternalTrack.PosKeys);

			// 회전 키 쓰기 (FQuat는 직접 직렬화)
			TArray<FQuat>& RotKeys = Track.InternalTrack.RotKeys;
			uint32 NumRotKeys = (uint32)RotKeys.Num();
			Writer << NumRotKeys;
			for (FQuat& Rot : RotKeys)
			{
				Writer << Rot.X << Rot.Y << Rot.Z << Rot.W;
			}

			// 스케일 키 쓰기
			Serialization::WriteArray(Writer, Track.InternalTrack.ScaleKeys);
		}

		Writer.Close();
		UE_LOG("Animation saved to cache: %s", CachePath.c_str());
		return true;
	}
	catch (const std::exception& e)
	{
		UE_LOG("Failed to save animation cache: %s", e.what());
		return false;
	}
}

UAnimSequence* FBXAnimationCache::LoadAnimationFromCache(const FString& CachePath)
{
	try
	{
		FWindowsBinReader Reader(CachePath);
		if (!Reader.IsOpen())
		{
			return nullptr;
		}

		// 새 애니메이션 시퀀스 생성
		UAnimSequence* Animation = NewObject<UAnimSequence>();

		// 애니메이션 이름 먼저 읽기
		FString AnimName;
		Serialization::ReadString(Reader, AnimName);
		Animation->ObjectName = FName(AnimName);

		UAnimDataModel* DataModel = Animation->GetDataModel();
		if (!DataModel)
		{
			return nullptr;
		}

		// 메타데이터 읽기
		float PlayLength;
		int32 FrameRate;
		int32 NumberOfFrames;
		int32 NumberOfKeys;

		Reader << PlayLength;
		Reader << FrameRate;
		Reader << NumberOfFrames;
		Reader << NumberOfKeys;

		DataModel->SetPlayLength(PlayLength);
		DataModel->SetFrameRate(FrameRate);
		DataModel->SetNumberOfFrames(NumberOfFrames);
		DataModel->SetNumberOfKeys(NumberOfKeys);

		// 호환성 검사를 위한 본 이름 읽기
		uint32 NumBoneNames;
		Reader << NumBoneNames;
		TArray<FName> BoneNames;
		for (uint32 i = 0; i < NumBoneNames; ++i)
		{
			FString BoneNameStr;
			Serialization::ReadString(Reader, BoneNameStr);
			BoneNames.Add(FName(BoneNameStr));
		}
		Animation->SetBoneNames(BoneNames);

		// 본 트랙 읽기
		uint32 NumTracks;
		Reader << NumTracks;

		for (uint32 i = 0; i < NumTracks; ++i)
		{
			// 본 이름 읽기
			FString BoneNameStr;
			Serialization::ReadString(Reader, BoneNameStr);
			FName BoneName(BoneNameStr);

			// 본 트랙 추가
			DataModel->AddBoneTrack(BoneName);

			// 위치 키 읽기
			TArray<FVector> PosKeys;
			Serialization::ReadArray(Reader, PosKeys);

			// 회전 키 읽기
			uint32 NumRotKeys;
			Reader << NumRotKeys;
			TArray<FQuat> RotKeys;
			RotKeys.resize(NumRotKeys);
			for (uint32 j = 0; j < NumRotKeys; ++j)
			{
				float X, Y, Z, W;
				Reader << X << Y << Z << W;
				RotKeys[j] = FQuat(X, Y, Z, W);
			}

			// 스케일 키 읽기
			TArray<FVector> ScaleKeys;
			Serialization::ReadArray(Reader, ScaleKeys);

			// 데이터 모델에 키 설정
			DataModel->SetBoneTrackKeys(BoneName, PosKeys, RotKeys, ScaleKeys);
		}

		Reader.Close();
		UE_LOG("Animation loaded from cache: %s (Name: %s)", CachePath.c_str(), AnimName.c_str());
		return Animation;
	}
	catch (const std::exception& e)
	{
		UE_LOG("Failed to load animation from cache: %s", e.what());
		return nullptr;
	}
}
