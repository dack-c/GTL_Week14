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


	UE_LOG("FBXMaterialLoader: ParseMaterial: Processing material '%s'", MaterialInfo.MaterialName.c_str());

	FbxProperty Property;

	Property = Material->FindProperty(FbxSurfaceMaterial::sDiffuse);
	MaterialInfo.DiffuseTextureFileName = ParseTexturePath(Property);
	UE_LOG("FBXMaterialLoader: ParseMaterial: Diffuse texture = '%s'", MaterialInfo.DiffuseTextureFileName.c_str());

	Property = Material->FindProperty(FbxSurfaceMaterial::sNormalMap);
	MaterialInfo.NormalTextureFileName = ParseTexturePath(Property);
	UE_LOG("FBXMaterialLoader: ParseMaterial: Normal texture = '%s'", MaterialInfo.NormalTextureFileName.c_str());

	Property = Material->FindProperty(FbxSurfaceMaterial::sAmbient);
	MaterialInfo.AmbientTextureFileName = ParseTexturePath(Property);
	UE_LOG("FBXMaterialLoader: ParseMaterial: Ambient texture = '%s'", MaterialInfo.AmbientTextureFileName.c_str());

	Property = Material->FindProperty(FbxSurfaceMaterial::sSpecular);
	MaterialInfo.SpecularTextureFileName = ParseTexturePath(Property);
	UE_LOG("FBXMaterialLoader: ParseMaterial: Specular texture = '%s'", MaterialInfo.SpecularTextureFileName.c_str());

	Property = Material->FindProperty(FbxSurfaceMaterial::sEmissive);
	MaterialInfo.EmissiveTextureFileName = ParseTexturePath(Property);
	UE_LOG("FBXMaterialLoader: ParseMaterial: Emissive texture = '%s'", MaterialInfo.EmissiveTextureFileName.c_str());

	Property = Material->FindProperty(FbxSurfaceMaterial::sTransparencyFactor);
	MaterialInfo.TransparencyTextureFileName = ParseTexturePath(Property);
	UE_LOG("FBXMaterialLoader: ParseMaterial: Transparency texture = '%s'", MaterialInfo.TransparencyTextureFileName.c_str());

	Property = Material->FindProperty(FbxSurfaceMaterial::sShininess);
	MaterialInfo.SpecularExponentTextureFileName = ParseTexturePath(Property);
	UE_LOG("FBXMaterialLoader: ParseMaterial: SpecularExponent texture = '%s'", MaterialInfo.SpecularExponentTextureFileName.c_str());

	UMaterial* Default = UResourceManager::GetInstance().GetDefaultMaterial();
	NewMaterial->SetMaterialInfo(MaterialInfo);
	NewMaterial->SetShader(Default->GetShader());
	NewMaterial->SetShaderMacros(Default->GetShaderMacros());
	NewMaterial->ResolveTextures(); // MaterialInfo의 텍스처 경로들을 기반으로 실제 UTexture 로드

	MaterialInfos.Add(MaterialInfo);
	UResourceManager::GetInstance().Add<UMaterial>(MaterialInfo.MaterialName, NewMaterial);
}

FString FBXMaterialLoader::ParseTexturePath(FbxProperty& Property)
{
	if (Property.IsValid())
	{
		int32 TextureCount = Property.GetSrcObjectCount<FbxFileTexture>();
		UE_LOG("FBXMaterialLoader: ParseTexturePath: Property='%s', TextureCount=%d", Property.GetName().Buffer(), TextureCount);

		if (TextureCount > 0)
		{
			FbxFileTexture* Texture = Property.GetSrcObject<FbxFileTexture>(0);
			if (Texture)
			{
				const char* AcpPath = Texture->GetRelativeFileName();
				bool bUsedRelative = (AcpPath && strlen(AcpPath) > 0);
				if (!bUsedRelative)
				{
					AcpPath = Texture->GetFileName();
				}

				if (!AcpPath)
				{
					UE_LOG("FBXMaterialLoader: ParseTexturePath: No filename found in FbxFileTexture");
					return FString();
				}

				FString TexturePath = ACPToUTF8(AcpPath);
				const FString& CurrentFbxBaseDir = UFbxLoader::GetInstance().GetCurrentFbxBaseDir();
				FString ResolvedPath = ResolveAssetRelativePath(TexturePath, CurrentFbxBaseDir);

				UE_LOG("FBXMaterialLoader: ParseTexturePath: RawPath='%s' (Relative=%d), BaseDir='%s', ResolvedPath='%s'",
					TexturePath.c_str(), bUsedRelative, CurrentFbxBaseDir.c_str(), ResolvedPath.c_str());

				// Smart Texture Matching: 경로 해석 실패 시 또는 파일이 존재하지 않으면 파일명으로 폴백 검색
				bool bNeedsFallback = ResolvedPath.empty();
				if (!bNeedsFallback)
				{
					// ResolvedPath가 있어도 실제 파일이 존재하지 않으면 폴백 필요
					FWideString WResolvedPath = UTF8ToWide(ResolvedPath);
					if (!std::filesystem::exists(WResolvedPath))
					{
						bNeedsFallback = true;
						UE_LOG("FBXMaterialLoader: ParseTexturePath: Resolved path does not exist, attempting smart matching");
					}
				}

				if (bNeedsFallback)
				{
					// RawPath에서 파일명(basename)만 추출 (경로 구분자 처리: / 또는 \)
					size_t LastSlash = TexturePath.find_last_of("/\\");
					FString FileName = (LastSlash != FString::npos) ? TexturePath.substr(LastSlash + 1) : TexturePath;

					const TMap<FString, FString>& TextureMap = UFbxLoader::GetTextureFileNameMap();
					auto It = TextureMap.find(FileName);
					if (It != TextureMap.end())
					{
						ResolvedPath = It->second;
						UE_LOG("FBXMaterialLoader: ParseTexturePath: Smart matching found '%s' -> '%s'", FileName.c_str(), ResolvedPath.c_str());
					}
					else
					{
						UE_LOG("FBXMaterialLoader: ParseTexturePath: Smart matching failed, filename '%s' not found in texture map", FileName.c_str());
					}
				}

				return ResolvedPath;
			}
		}
		else
		{
			UE_LOG("FBXMaterialLoader: ParseTexturePath: Property '%s' has no FbxFileTexture objects", Property.GetName().Buffer());
		}
	}
	else
	{
		UE_LOG("FBXMaterialLoader: ParseTexturePath: Property is invalid");
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
