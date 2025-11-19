#pragma once
#include "fbxsdk.h"
#include "SkeletalMesh.h"
#include "Material.h"

class FBXMaterialLoader
{
public:
	// FBX에서 머티리얼을 파싱하여 MaterialInfo 생성
	static void ParseMaterial(FbxSurfaceMaterial* Material, FMaterialInfo& MaterialInfo, TArray<FMaterialInfo>& MaterialInfos);

	// FBX 프로퍼티에서 텍스처 경로 파싱
	static FString ParseTexturePath(FbxProperty& Property);

	// 속성 타입 이름 가져오기 (디버깅/로깅용)
	static FbxString GetAttributeTypeName(FbxNodeAttribute* InAttribute);
};
