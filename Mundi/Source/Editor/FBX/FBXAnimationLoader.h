#pragma once
#include "fbxsdk.h"
#include "SkeletalMesh.h"

class UAnimSequence;

class FBXAnimationLoader
{
public:
	// FBX Scene에서 모든 애니메이션 처리
	static void ProcessAnimations(FbxScene* Scene, const FSkeletalMeshData& MeshData, const FString& FilePath, TArray<UAnimSequence*>& OutAnimations);

	// 특정 본의 애니메이션 데이터 추출
	static void ExtractBoneAnimation(FbxNode* BoneNode, FbxAnimLayer* AnimLayer, FbxTime StartTime, FbxLongLong FrameCount, FbxTime::EMode TimeMode, TArray<FVector>& OutPositions, TArray<FQuat>& OutRotations, TArray<FVector>& OutScales, const FbxAMatrix& ArmatureTransform, bool bIsRootBone);

	// 노드에 애니메이션 커브가 있는지 확인
	static bool NodeHasAnimation(FbxNode* Node, FbxAnimLayer* AnimLayer);

	// 본 이름에서 FbxNode 포인터로의 맵 구축
	static void BuildBoneNodeMap(FbxNode* RootNode, TMap<FName, FbxNode*>& OutBoneNodeMap);
};
