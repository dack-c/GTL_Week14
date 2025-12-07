#include "pch.h"
#include "FBXMaterialLoader.h"

#include "FbxLoader.h"
#include "ResourceManager.h"

// 머티리얼 파싱해서 FMaterialInfo에 매핑
void FBXMaterialLoader::ParseMaterial(FbxSurfaceMaterial* Material, FMaterialInfo& MaterialInfo, TArray<FMaterialInfo>& MaterialInfos)
{

	UMaterial* NewMaterial = NewObject<UMaterial>();

	// FbxPropertyT : 타입에 대해 애니메이션과 연결 지원(키프레임마다 타입 변경 등)
	FbxPropertyT<FbxDouble3> Double3Prop;
	FbxPropertyT<FbxDouble> DoubleProp;

	MaterialInfo.MaterialName = Material->GetName();
	// PBR 제외하고 Phong, Lambert 머티리얼만 처리함. 
	if (Material->GetClassId().Is(FbxSurfacePhong::ClassId))
	{
		FbxSurfacePhong* SurfacePhong = (FbxSurfacePhong*)Material;

		Double3Prop = SurfacePhong->Diffuse;
		MaterialInfo.DiffuseColor = FVector(Double3Prop.Get()[0], Double3Prop.Get()[1], Double3Prop.Get()[2]);

		Double3Prop = SurfacePhong->Ambient;
		MaterialInfo.AmbientColor = FVector(Double3Prop.Get()[0], Double3Prop.Get()[1], Double3Prop.Get()[2]);

		// SurfacePhong->Reflection : 환경 반사, 퐁 모델에선 필요없음
		Double3Prop = SurfacePhong->Specular;
		DoubleProp = SurfacePhong->SpecularFactor;
		MaterialInfo.SpecularColor = FVector(Double3Prop.Get()[0], Double3Prop.Get()[1], Double3Prop.Get()[2]) * DoubleProp.Get();

		// HDR 안 써서 의미 없음
	/*	Double3Prop = SurfacePhong->Emissive;
		MaterialInfo.EmissiveColor = FVector(Double3Prop.Get()[0], Double3Prop.Get()[1], Double3Prop.Get()[2]);*/

		DoubleProp = SurfacePhong->Shininess;
		MaterialInfo.SpecularExponent = DoubleProp.Get();

		DoubleProp = SurfacePhong->TransparencyFactor;
		MaterialInfo.Transparency = DoubleProp.Get();
	}
	else if (Material->GetClassId().Is(FbxSurfaceLambert::ClassId))
	{
		FbxSurfaceLambert* SurfacePhong = (FbxSurfaceLambert*)Material;

		Double3Prop = SurfacePhong->Diffuse;
		MaterialInfo.DiffuseColor = FVector(Double3Prop.Get()[0], Double3Prop.Get()[1], Double3Prop.Get()[2]);

		Double3Prop = SurfacePhong->Ambient;
		MaterialInfo.AmbientColor = FVector(Double3Prop.Get()[0], Double3Prop.Get()[1], Double3Prop.Get()[2]);

		// HDR 안 써서 의미 없음
	/*	Double3Prop = SurfacePhong->Emissive;
		MaterialInfo.EmissiveColor = FVector(Double3Prop.Get()[0], Double3Prop.Get()[1], Double3Prop.Get()[2]);*/

		DoubleProp = SurfacePhong->TransparencyFactor;
		MaterialInfo.Transparency = DoubleProp.Get();
	}


	FbxProperty Property;

	Property = Material->FindProperty(FbxSurfaceMaterial::sDiffuse);
	MaterialInfo.DiffuseTextureFileName = ParseTexturePath(Property);

	Property = Material->FindProperty(FbxSurfaceMaterial::sNormalMap);
	MaterialInfo.NormalTextureFileName = ParseTexturePath(Property);

	Property = Material->FindProperty(FbxSurfaceMaterial::sAmbient);
	MaterialInfo.AmbientTextureFileName = ParseTexturePath(Property);

	Property = Material->FindProperty(FbxSurfaceMaterial::sSpecular);
	MaterialInfo.SpecularTextureFileName = ParseTexturePath(Property);

	Property = Material->FindProperty(FbxSurfaceMaterial::sEmissive);
	MaterialInfo.EmissiveTextureFileName = ParseTexturePath(Property);

	Property = Material->FindProperty(FbxSurfaceMaterial::sTransparencyFactor);
	MaterialInfo.TransparencyTextureFileName = ParseTexturePath(Property);

	Property = Material->FindProperty(FbxSurfaceMaterial::sShininess);
	MaterialInfo.SpecularExponentTextureFileName = ParseTexturePath(Property);

	UMaterial* Default = UResourceManager::GetInstance().GetDefaultMaterial();
	NewMaterial->SetMaterialInfo(MaterialInfo);
	NewMaterial->SetShader(Default->GetShader());
	NewMaterial->SetShaderMacros(Default->GetShaderMacros());

	MaterialInfos.Add(MaterialInfo);
	UResourceManager::GetInstance().Add<UMaterial>(MaterialInfo.MaterialName, NewMaterial);
}

// 여러 디렉토리에서 텍스처 파일을 찾는 헬퍼 함수
static FString FindTextureFile(const FString& InTexturePath, const FString& InFbxBaseDir)
{
	if (InTexturePath.empty())
		return FString();

	// 1. 먼저 원본 경로로 해석 시도
	FString ResolvedPath = ResolveAssetRelativePath(InTexturePath, InFbxBaseDir);
	FWideString WResolvedPath = UTF8ToWide(ResolvedPath);

	std::error_code ec;
	if (fs::exists(WResolvedPath, ec) && !ec)
	{
		// 원본 경로가 유효하면 그대로 반환
		return ResolvedPath;
	}

	// 2. 원본 경로가 유효하지 않으면 파일명만 추출
	FWideString WPath = UTF8ToWide(InTexturePath);
	fs::path OriginalPath(WPath);
	FString FileName = WideToUTF8(OriginalPath.filename().wstring());

	if (FileName.empty())
		return FString();

	// 3. 검색할 디렉토리 목록
	TArray<FString> SearchDirs;
	SearchDirs.Add(InFbxBaseDir);                    // FBX 파일이 있는 디렉토리
	SearchDirs.Add(GDataDir + "/Textures");          // Data/Textures
	SearchDirs.Add(GDataDir + "/Model");             // Data/Model
	SearchDirs.Add(GDataDir + "/Untracked");         // Data/Untracked

	// 4. 각 디렉토리에서 파일 찾기
	for (const FString& Dir : SearchDirs)
	{
		FString CandidatePath = Dir + "/" + FileName;
		FWideString WCandidatePath = UTF8ToWide(CandidatePath);

		if (fs::exists(WCandidatePath, ec) && !ec)
		{
			UE_LOG("[FBX Texture] Found texture: %s -> %s", InTexturePath.c_str(), CandidatePath.c_str());
			return NormalizePath(CandidatePath);
		}
	}

	// 5. 찾지 못했을 경우 경고 로그
	UE_LOG("[FBX Texture] WARNING: Could not find texture file: %s (filename: %s)", InTexturePath.c_str(), FileName.c_str());

	// 6. 원본 경로 반환 (기존 동작 유지)
	return ResolvedPath;
}

FString FBXMaterialLoader::ParseTexturePath(FbxProperty& Property)
{
	if (Property.IsValid())
	{
		if (Property.GetSrcObjectCount<FbxFileTexture>() > 0)
		{
			FbxFileTexture* Texture = Property.GetSrcObject<FbxFileTexture>(0);
			if (Texture)
			{
				const char* AcpPath = Texture->GetRelativeFileName();
				if (!AcpPath || strlen(AcpPath) == 0)
				{
					AcpPath = Texture->GetFileName();
				}

				if (!AcpPath)
				{
					return FString();
				}

				FString TexturePath = ACPToUTF8(AcpPath);
				const FString& CurrentFbxBaseDir = UFbxLoader::GetInstance().GetCurrentFbxBaseDir();

				// FindTextureFile을 사용하여 텍스처 파일 자동 탐색
				return FindTextureFile(TexturePath, CurrentFbxBaseDir);
			}
		}
	}
	return FString();
}

FbxString FBXMaterialLoader::GetAttributeTypeName(FbxNodeAttribute* InAttribute)
{
	// 테스트코드
	// Attribute타입에 대한 자료형, 이것으로 Skeleton만 빼낼 수 있을 듯
	/*FbxNodeAttribute::EType Type = InAttribute->GetAttributeType();
	switch (Type) {
	case FbxNodeAttribute::eUnknown: return "unidentified";
	case FbxNodeAttribute::eNull: return "null";
	case FbxNodeAttribute::eMarker: return "marker";
	case FbxNodeAttribute::eSkeleton: return "skeleton";
	case FbxNodeAttribute::eMesh: return "mesh";
	case FbxNodeAttribute::eNurbs: return "nurbs";
	case FbxNodeAttribute::ePatch: return "patch";
	case FbxNodeAttribute::eCamera: return "camera";
	case FbxNodeAttribute::eCameraStereo: return "stereo";
	case FbxNodeAttribute::eCameraSwitcher: return "camera switcher";
	case FbxNodeAttribute::eLight: return "light";
	case FbxNodeAttribute::eOpticalReference: return "optical reference";
	case FbxNodeAttribute::eOpticalMarker: return "marker";
	case FbxNodeAttribute::eNurbsCurve: return "nurbs curve";
	case FbxNodeAttribute::eTrimNurbsSurface: return "trim nurbs surface";
	case FbxNodeAttribute::eBoundary: return "boundary";
	case FbxNodeAttribute::eNurbsSurface: return "nurbs surface";
	case FbxNodeAttribute::eShape: return "shape";
	case FbxNodeAttribute::eLODGroup: return "lodgroup";
	case FbxNodeAttribute::eSubDiv: return "subdiv";
	default: return "unknown";
	}*/
	return "test";
}
