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


protected:
	~UFbxLoader() override;
private:
	UFbxLoader(const UFbxLoader&) = delete;
	UFbxLoader& operator=(const UFbxLoader&) = delete;


	// bin파일 저장용
	TArray<FMaterialInfo> MaterialInfos;
	FbxManager* SdkManager = nullptr;

	// 비-스켈레톤 부모 노드(예: Armature)의 로컬 트랜스폼 저장 (애니메이션 보정용)
	TMap<const FbxNode*, FbxAMatrix> NonSkeletonParentTransforms;
};
