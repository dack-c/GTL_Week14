#include "pch.h"
#include "StaticMesh.h"
#include "StaticMeshComponent.h"
#include "ObjManager.h"
#include "ResourceManager.h"
#include "Source/Editor/FBX/FbxLoader.h"
#include "Source/Runtime/Engine/Physics/BodySetup.h"
#include "Source/Runtime/Core/Misc/PathUtils.h"
#include "Source/Runtime/Core/Misc/WindowsBinReader.h"
#include "Source/Runtime/Core/Misc/WindowsBinWriter.h"
#include <filesystem>

IMPLEMENT_CLASS(UStaticMesh)

namespace
{
    // FSkeletalMeshData를 FStaticMesh로 변환 (본 정보 제거)
    FStaticMesh* ConvertSkeletalToStaticMesh(const FSkeletalMeshData& SkeletalData)
    {
        FStaticMesh* StaticMesh = new FStaticMesh();

        // 정점 데이터 변환 (FSkinnedVertex -> FNormalVertex)
        StaticMesh->Vertices.reserve(SkeletalData.Vertices.size());
        for (const auto& SkinnedVtx : SkeletalData.Vertices)
        {
            FNormalVertex NormalVtx;
            NormalVtx.pos = SkinnedVtx.Position;
            NormalVtx.normal = SkinnedVtx.Normal;
            NormalVtx.tex = SkinnedVtx.UV;
            NormalVtx.Tangent = SkinnedVtx.Tangent;
            NormalVtx.color = SkinnedVtx.Color;
            StaticMesh->Vertices.push_back(NormalVtx);
        }

        // 인덱스 복사
        StaticMesh->Indices = SkeletalData.Indices;

        // 그룹 정보 복사
        StaticMesh->GroupInfos = SkeletalData.GroupInfos;
        StaticMesh->bHasMaterial = SkeletalData.bHasMaterial;

        // 캐시 경로 복사
        StaticMesh->CacheFilePath = SkeletalData.CacheFilePath;

        return StaticMesh;
    }
}

UStaticMesh::~UStaticMesh()
{
    ReleaseResources();
}

bool UStaticMesh::Load(const FString& InFilePath, ID3D11Device* InDevice, EVertexLayoutType InVertexType)
{
    assert(InDevice);

    SetVertexType(InVertexType);

    // 파일 확장자 확인
    std::filesystem::path FilePath(InFilePath);
    FString Extension = FilePath.extension().string();
    std::transform(Extension.begin(), Extension.end(), Extension.begin(), ::tolower);

    if (Extension == ".fbx")
    {
        // FBX 파일 로드
        FSkeletalMeshData* SkeletalData = UFbxLoader::GetInstance().LoadFbxMeshAsset(InFilePath);

        if (SkeletalData->Vertices.empty() || SkeletalData->Indices.empty())
        {
            UE_LOG("ERROR: Failed to load FBX mesh from '%s'", InFilePath.c_str());
            delete SkeletalData;
            return false;
        }

        // SkeletalMeshData를 StaticMesh로 변환
        StaticMeshAsset = ConvertSkeletalToStaticMesh(*SkeletalData);
        StaticMeshAsset->PathFileName = InFilePath;

        // FBX 메시를 ObjManager 캐시에 등록 (메모리 관리)
        FObjManager::RegisterStaticMeshAsset(InFilePath, StaticMeshAsset);
        delete SkeletalData;
    }
    else
    {
        // OBJ 파일 로드 (기존 방식)
        StaticMeshAsset = FObjManager::LoadObjStaticMeshAsset(InFilePath);
    }

    // 빈 버텍스, 인덱스로 버퍼 생성 방지
    if (StaticMeshAsset && 0 < StaticMeshAsset->Vertices.size() && 0 < StaticMeshAsset->Indices.size())
    {
        CacheFilePath = StaticMeshAsset->CacheFilePath;
        CreateVertexBuffer(StaticMeshAsset, InDevice, InVertexType);
        CreateIndexBuffer(StaticMeshAsset, InDevice);
        CreateLocalBound(StaticMeshAsset);
        CreateBodySetupFromBounds();
        InitConvexMesh();
        InitTriangleMesh();
        VertexCount = static_cast<uint32>(StaticMeshAsset->Vertices.size());
        IndexCount = static_cast<uint32>(StaticMeshAsset->Indices.size());
    }
    return true;
}

bool UStaticMesh::Load(FMeshData* InData, ID3D11Device* InDevice, EVertexLayoutType InVertexType)
{
    SetVertexType(InVertexType);

    if (VertexBuffer)
    {
        VertexBuffer->Release();
        VertexBuffer = nullptr;
    }
    if (IndexBuffer)
    {
        IndexBuffer->Release();
        IndexBuffer = nullptr;
    }

    CreateVertexBuffer(InData, InDevice, InVertexType);
    CreateIndexBuffer(InData, InDevice);
    CreateLocalBound(InData);
    CreateBodySetupFromBounds();

    VertexCount = static_cast<uint32>(InData->Vertices.size());
    IndexCount = static_cast<uint32>(InData->Indices.size());

    return true;
}

void UStaticMesh::SetVertexType(EVertexLayoutType InVertexType)
{
    VertexType = InVertexType;

    uint32 Stride = 0;
    switch (InVertexType)
    {
    case EVertexLayoutType::PositionColor:
        Stride = sizeof(FVertexSimple);
        break;
    case EVertexLayoutType::PositionColorTexturNormal:
        Stride = sizeof(FVertexDynamic);
        break;
    case EVertexLayoutType::PositionTextBillBoard:
        Stride = sizeof(FBillboardVertexInfo_GPU);
        break;
    case EVertexLayoutType::PositionBillBoard:
        Stride = sizeof(FBillboardVertex);
        break;
    default:
        assert(false && "Unknown vertex type!");
    }

    VertexStride = Stride;
}

void UStaticMesh::CreateVertexBuffer(FMeshData* InMeshData, ID3D11Device* InDevice, EVertexLayoutType InVertexType)
{
    HRESULT hr;
    hr = D3D11RHI::CreateVertexBuffer<FVertexDynamic>(InDevice, *InMeshData, &VertexBuffer);
    assert(SUCCEEDED(hr));
}

void UStaticMesh::CreateVertexBuffer(FStaticMesh* InStaticMesh, ID3D11Device* InDevice, EVertexLayoutType InVertexType)
{
    HRESULT hr;
    hr = D3D11RHI::CreateVertexBuffer<FVertexDynamic>(InDevice, InStaticMesh->Vertices, &VertexBuffer);
    assert(SUCCEEDED(hr));
}

void UStaticMesh::CreateIndexBuffer(FMeshData* InMeshData, ID3D11Device* InDevice)
{
    HRESULT hr = D3D11RHI::CreateIndexBuffer(InDevice, InMeshData, &IndexBuffer);
    assert(SUCCEEDED(hr));
}

void UStaticMesh::CreateIndexBuffer(FStaticMesh* InStaticMesh, ID3D11Device* InDevice)
{
    HRESULT hr = D3D11RHI::CreateIndexBuffer(InDevice, InStaticMesh, &IndexBuffer);
    assert(SUCCEEDED(hr));
}

void UStaticMesh::CreateLocalBound(const FMeshData* InMeshData)
{
    TArray<FVector> Verts = InMeshData->Vertices;
    FVector Min = Verts[0];
    FVector Max = Verts[0];
    for (FVector Vertex : Verts)
    {
        Min = Min.ComponentMin(Vertex);
        Max = Max.ComponentMax(Vertex);
    }
    LocalBound = FAABB(Min, Max);
}

void UStaticMesh::CreateLocalBound(const FStaticMesh* InStaticMesh)
{
    TArray<FNormalVertex> Verts = InStaticMesh->Vertices;
    FVector Min = Verts[0].pos;
    FVector Max = Verts[0].pos;
    for (FNormalVertex Vertex : Verts)
    {
        FVector Pos = Vertex.pos;
        Min = Min.ComponentMin(Pos);
        Max = Max.ComponentMax(Pos);
    }
    LocalBound = FAABB(Min, Max);
}

void UStaticMesh::CreateBodySetupFromBounds()
{
    // 기존 BodySetup이 있으면 삭제
    if (BodySetup)
    {
        ObjectFactory::DeleteObject(BodySetup);
        BodySetup = nullptr;
    }

    // BodySetup 생성
    BodySetup = ObjectFactory::NewObject<UBodySetup>();
	BodySetup->AggGeom.ConvexElements.Add(FKConvexElem());
}

void UStaticMesh::InitConvexMesh()
{
	if (!BodySetup || BodySetup->AggGeom.ConvexElements.IsEmpty() || !StaticMeshAsset || StaticMeshAsset->Vertices.empty())
	{
		return;
	}

	FString convexCachePath = ConvertDataPathToCachePath(GetAssetPathFileName()) + ".convex.bin";

	bool bShouldRegenerate = true;
	if (std::filesystem::exists(convexCachePath))
	{
		try
		{
			auto cacheTime = std::filesystem::last_write_time(convexCachePath);
			auto originalTime = std::filesystem::last_write_time(GetAssetPathFileName());
			if (cacheTime >= originalTime)
			{
				bShouldRegenerate = false;
			}
		}
		catch (const std::filesystem::filesystem_error& e)
		{
			UE_LOG("Filesystem error during cache validation: %s. Forcing regeneration.", e.what());
			bShouldRegenerate = true;
		}
	}

	FKConvexElem& convexElem = BodySetup->AggGeom.ConvexElements[0];

	if (!bShouldRegenerate)
	{
		try
		{
			FWindowsBinReader reader(convexCachePath);
			if (reader.IsOpen())
			{
				Serialization::ReadArray(reader, convexElem.CookedData);
				reader.Close();
				UE_LOG("Loaded convex mesh from cache: %s", convexCachePath.c_str());
			}
			else
			{
				bShouldRegenerate = true;
			}
		}
		catch (const std::exception& e)
		{
			UE_LOG("Error loading convex cache: %s. Forcing regeneration.", e.what());
			bShouldRegenerate = true;
		}
	}

	if (bShouldRegenerate)
	{
		// ===== OPTIMIZATION: 복잡한 메시는 convex cooking 스킵 =====
		// Convex hull 계산은 O(n^2) 복잡도, 10만 정점 이상은 수 분~수 시간 소요
		const uint32 MAX_CONVEX_VERTICES = 100000;
		if (StaticMeshAsset->Vertices.size() > MAX_CONVEX_VERTICES)
		{
			UE_LOG("Mesh too complex for convex cooking (%zu vertices > %d limit), skipping: %s",
				   StaticMeshAsset->Vertices.size(), MAX_CONVEX_VERTICES, GetAssetPathFileName().c_str());
			UE_LOG("Mesh will load and render, but won't have physics collision.");
			return; // Convex cooking만 스킵, 메시 렌더링은 정상 진행
		}
		// ===== END OPTIMIZATION =====

		UE_LOG("Cooking convex mesh for: %s", GetAssetPathFileName().c_str());

		physx::PxDefaultAllocator      gAllocator;
		physx::PxDefaultErrorCallback  gErrorCallback;
		physx::PxFoundation*           gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
		if (!gFoundation) { UE_LOG("PxCreateFoundation failed!"); return; }

		physx::PxPhysics*              gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, physx::PxTolerancesScale(), false, nullptr);
		if (!gPhysics) { UE_LOG("PxCreatePhysics failed!"); gFoundation->release(); return; }

		physx::PxCooking*              gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, physx::PxCookingParams(gPhysics->getTolerancesScale()));
		if (!gCooking) { UE_LOG("PxCreateCooking failed!"); gPhysics->release(); gFoundation->release(); return; }


		TArray<physx::PxVec3> pxVertices;
		pxVertices.Reserve(StaticMeshAsset->Vertices.size());
		for (const auto& vert : StaticMeshAsset->Vertices)
		{
			pxVertices.Add(physx::PxVec3(vert.pos.X, vert.pos.Y, vert.pos.Z));
		}

		physx::PxConvexMeshDesc convexDesc;
		convexDesc.points.count = static_cast<uint32>(pxVertices.size());
		convexDesc.points.stride = sizeof(physx::PxVec3);
		convexDesc.points.data = pxVertices.GetData();
        convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;

		physx::PxDefaultMemoryOutputStream buf;
		physx::PxConvexMeshCookingResult::Enum result;
		if (!gCooking->cookConvexMesh(convexDesc, buf, &result))
		{
			UE_LOG("Failed to cook convex mesh for %s", GetAssetPathFileName().c_str());
			gCooking->release();
			gPhysics->release();
			gFoundation->release();
			return;
		}

		convexElem.CookedData.SetNum(buf.getSize());
		memcpy(convexElem.CookedData.GetData(), buf.getData(), buf.getSize());

		FWindowsBinWriter writer(convexCachePath);

        Serialization::WriteArray(writer, convexElem.CookedData);
        writer.Close();
        UE_LOG("Saved cooked convex mesh to cache: %s", convexCachePath.c_str());

		gCooking->release();
		gPhysics->release();
		gFoundation->release();
	}
}

void UStaticMesh::InitTriangleMesh()
{
	if (!BodySetup || !StaticMeshAsset || StaticMeshAsset->Vertices.empty() || StaticMeshAsset->Indices.empty())
	{
		return;
	}

	FString trimeshCachePath = ConvertDataPathToCachePath(GetAssetPathFileName()) + ".trimesh.bin";

	bool bShouldRegenerate = true;
	if (std::filesystem::exists(trimeshCachePath))
	{
		try
		{
			auto cacheTime = std::filesystem::last_write_time(trimeshCachePath);
			auto originalTime = std::filesystem::last_write_time(GetAssetPathFileName());
			if (cacheTime >= originalTime)
			{
				bShouldRegenerate = false;
			}
		}
		catch (const std::filesystem::filesystem_error& e)
		{
			UE_LOG("Filesystem error during cache validation: %s. Forcing regeneration.", e.what());
			bShouldRegenerate = true;
		}
	}

	FKTriangleMeshElem TriMeshElem;

	if (!bShouldRegenerate)
	{
		try
		{
			FWindowsBinReader reader(trimeshCachePath);
			if (reader.IsOpen())
			{
				Serialization::ReadArray(reader, TriMeshElem.CookedData);
				reader.Close();
				UE_LOG("Loaded triangle mesh from cache: %s", trimeshCachePath.c_str());
			}
			else
			{
				bShouldRegenerate = true;
			}
		}
		catch (const std::exception& e)
		{
			UE_LOG("Error loading triangle mesh cache: %s. Forcing regeneration.", e.what());
			bShouldRegenerate = true;
		}
	}

	if (bShouldRegenerate)
	{
		// Triangle Mesh는 복잡한 메시도 처리 가능하지만, 너무 큰 메시는 경고
		const uint32 MAX_TRIMESH_VERTICES = 500000;
		if (StaticMeshAsset->Vertices.size() > MAX_TRIMESH_VERTICES)
		{
			UE_LOG("Warning: Very large mesh for triangle collision (%zu vertices), may impact performance: %s",
				   StaticMeshAsset->Vertices.size(), GetAssetPathFileName().c_str());
		}

		UE_LOG("Cooking triangle mesh for: %s", GetAssetPathFileName().c_str());

		physx::PxDefaultAllocator      gAllocator;
		physx::PxDefaultErrorCallback  gErrorCallback;
		physx::PxFoundation*           gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
		if (!gFoundation) { UE_LOG("PxCreateFoundation failed!"); return; }

		physx::PxPhysics*              gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, physx::PxTolerancesScale(), false, nullptr);
		if (!gPhysics) { UE_LOG("PxCreatePhysics failed!"); gFoundation->release(); return; }

		physx::PxCooking*              gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, physx::PxCookingParams(gPhysics->getTolerancesScale()));
		if (!gCooking) { UE_LOG("PxCreateCooking failed!"); gPhysics->release(); gFoundation->release(); return; }

		// 정점 데이터 변환
		TArray<physx::PxVec3> pxVertices;
		pxVertices.Reserve(StaticMeshAsset->Vertices.size());
		for (const auto& vert : StaticMeshAsset->Vertices)
		{
			pxVertices.Add(physx::PxVec3(vert.pos.X, vert.pos.Y, vert.pos.Z));
		}

		// Triangle Mesh Descriptor 설정
		physx::PxTriangleMeshDesc triMeshDesc;
		triMeshDesc.points.count = static_cast<uint32>(pxVertices.size());
		triMeshDesc.points.stride = sizeof(physx::PxVec3);
		triMeshDesc.points.data = pxVertices.GetData();

		triMeshDesc.triangles.count = static_cast<uint32>(StaticMeshAsset->Indices.size() / 3);
		triMeshDesc.triangles.stride = 3 * sizeof(uint32);
		triMeshDesc.triangles.data = StaticMeshAsset->Indices.GetData();

		physx::PxDefaultMemoryOutputStream buf;
		physx::PxTriangleMeshCookingResult::Enum result;
		if (!gCooking->cookTriangleMesh(triMeshDesc, buf, &result))
		{
			UE_LOG("Failed to cook triangle mesh for %s", GetAssetPathFileName().c_str());
			gCooking->release();
			gPhysics->release();
			gFoundation->release();
			return;
		}

		TriMeshElem.CookedData.SetNum(buf.getSize());
		memcpy(TriMeshElem.CookedData.GetData(), buf.getData(), buf.getSize());

		FWindowsBinWriter writer(trimeshCachePath);
		Serialization::WriteArray(writer, TriMeshElem.CookedData);
		writer.Close();
		UE_LOG("Saved cooked triangle mesh to cache: %s", trimeshCachePath.c_str());

		gCooking->release();
		gPhysics->release();
		gFoundation->release();
	}

	// BodySetup에 Triangle Mesh 추가
	BodySetup->AggGeom.TriangleMeshElements.Add(TriMeshElem);
}

void UStaticMesh::ReleaseResources()
{
    if (VertexBuffer)
    {
        VertexBuffer->Release();
        VertexBuffer = nullptr;
    }
    if (IndexBuffer)
    {
        IndexBuffer->Release();
        IndexBuffer = nullptr;
    }
    if (BodySetup)
    {
        ObjectFactory::DeleteObject(BodySetup);
        BodySetup = nullptr;
    }
}

bool UStaticMesh::GetCollisionMeshData(TArray<FVector>& OutVertices, TArray<uint32>& OutIndices) const
{
    if (!StaticMeshAsset)
    {
        return false;
    }

    OutVertices.Empty();
    OutIndices.Empty();

    // FNormalVertex에서 Position만 추출
    OutVertices.reserve(StaticMeshAsset->Vertices.size());
    for (const FNormalVertex& Vtx : StaticMeshAsset->Vertices)
    {
        OutVertices.Add(Vtx.pos);
    }

    // 인덱스 복사
    OutIndices = StaticMeshAsset->Indices;

    return true;
}