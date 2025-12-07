#pragma once
#include "fbxsdk.h"
#include "SkeletalMesh.h"

class FBXMeshLoader
{
public:
	// 노드와 그 자식들로부터 메쉬 데이터 로드
	static void LoadMeshFromNode(FbxNode* InNode, FSkeletalMeshData& MeshData, TMap<int32, TArray<uint32>>& MaterialGroupIndexList, TMap<FbxNode*, int32>& BoneToIndex, TMap<FbxSurfaceMaterial*, int32>& MaterialToIndex, TArray<FMaterialInfo>& MaterialInfos, TMap<FSkinnedVertex, uint32>& IndexMap);

	// 특정 속성으로부터 메쉬 로드
	static void LoadMeshFromAttribute(FbxNodeAttribute* InAttribute, FSkeletalMeshData& MeshData);

	// 메쉬 지오메트리, UV, 노말, 탄젠트 등 로드
	static void LoadMesh(FbxMesh* InMesh, FSkeletalMeshData& MeshData, TMap<int32, TArray<uint32>>& MaterialGroupIndexList, TMap<FbxNode*, int32>& BoneToIndex, TArray<int32> MaterialSlotToIndex, TMap<FSkinnedVertex, uint32>& IndexMap, int32 DefaultMaterialIndex = 0);
};
