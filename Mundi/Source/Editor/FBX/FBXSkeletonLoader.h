#pragma once
#include "fbxsdk.h"
#include "SkeletalMesh.h"

class FBXSkeletonLoader
{
public:
	// 헬퍼: 중간 노드(예: Armature)를 건너뛰고 eSkeleton을 찾을 때까지 노드를 재귀적으로 탐색
	static void LoadSkeletonHierarchy(FbxNode* InNode, FSkeletalMeshData& MeshData, int32 ParentNodeIndex, TMap<FbxNode*, int32>& BoneToIndex);

	// Skeleton은 계층구조까지 표현해야하므로 깊이 우선 탐색, ParentNodeIndex 명시
	static void LoadSkeletonFromNode(FbxNode* InNode, FSkeletalMeshData& MeshData, int32 ParentNodeIndex, TMap<FbxNode*, int32>& BoneToIndex);

	// 여러 루트 본이 있으면 가상 루트 생성
	static void EnsureSingleRootBone(FSkeletalMeshData& MeshData);
};
