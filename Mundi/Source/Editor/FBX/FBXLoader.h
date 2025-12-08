#pragma once
#include "Object.h"
#include "fbxsdk.h"
#include "FBXSceneUtilities.h"
#include "FBXMaterialLoader.h"
#include "FBXSkeletonLoader.h"
#include "FBXMeshLoader.h"
#include "FBXAnimationLoader.h"
#include "FBXAnimationCache.h"

class UAnimSequence;

class UFbxLoader : public UObject
{
public:

	DECLARE_CLASS(UFbxLoader, UObject)
	static UFbxLoader& GetInstance();
	UFbxLoader();

	static void PreLoad();

	USkeletalMesh* LoadFbxMesh(const FString& FilePath);

	FSkeletalMeshData* LoadFbxMeshAsset(const FString& FilePath);

	const FString& GetCurrentFbxBaseDir() const { return CurrentFbxBaseDir; }

	// 텍스처 파일명으로 전체 경로를 찾는 맵 (Smart Texture Matching)
	static const TMap<FString, FString>& GetTextureFileNameMap() { return TextureFileNameMap; }

protected:
	~UFbxLoader() override;
private:
	UFbxLoader(const UFbxLoader&) = delete;
	UFbxLoader& operator=(const UFbxLoader&) = delete;


	// bin파일 저장용
	TArray<FMaterialInfo> MaterialInfos;
	FbxManager* SdkManager = nullptr;

	/** 현재 로드 중인 FBX 파일의 상위 디렉토리 (UTF-8) */
	FString CurrentFbxBaseDir;

	// 비-스켈레톤 부모 노드(예: Armature)의 로컬 트랜스폼 저장 (애니메이션 보정용)
	TMap<const FbxNode*, FbxAMatrix> NonSkeletonParentTransforms;

	// Smart Texture Matching: 파일명 → 전체 경로 맵 (Key: basename, Value: full path)
	static TMap<FString, FString> TextureFileNameMap;
};
