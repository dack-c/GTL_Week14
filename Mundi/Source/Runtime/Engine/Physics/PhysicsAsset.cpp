#include "pch.h"
#include "PhysicsAsset.h"
#include "BodySetup.h"
#include "../Animation/AnimationAsset.h"
#include "SkeletalMeshComponent.h"
IMPLEMENT_CLASS(UPhysicsAsset)

namespace
{
    inline FVector InverseTransformPositionNoScale(const FTransform& Transform, const FVector& Point)
    {
        // Position -> subtract translation, then rotate by inverse
        const FVector Local = Point - Transform.Translation;
        return Transform.Rotation.Inverse().RotateVector(Local);
    }

    inline FQuat InverseTransformRotationNoScale(const FTransform& Transform, const FQuat& Rotation)
    {
        return Transform.Rotation.Inverse() * Rotation;
    }
}

UPhysicsAsset::~UPhysicsAsset()
{
    CurrentSkeletal = nullptr;
}

void UPhysicsAsset::BuildRuntimeCache()
{
    // 1) Body 이름 -> 인덱스 맵
    BuildBodySetupIndexMap();

    // 2) 각 BodySetup 캐시 (Bounds, Mass 등)
    for (UBodySetup* Body : BodySetups)
    {
        if (!Body) continue;
        if (Body->bCachedDataDirty)
        {
            Body->BuildCachedData();
            Body->bCachedDataDirty = false;
        }
    }
}

void UPhysicsAsset::BuildBodySetupIndexMap()
{
	BodySetupIndexMap.Empty();

	for (int32 Index = 0; Index < BodySetups.Num(); ++Index)
	{
		if (BodySetups[Index])
		{
			BodySetupIndexMap.Add(BodySetups[Index]->BoneName, Index);
		}
	}
}

void UPhysicsAsset::BuildBodyGroups(const FSkeleton* Skeleton, TArray<FBodyBoneGroup>& OutBodyGroups) const
{
    OutBodyGroups.Empty();

    if (!Skeleton || !CurrentSkeletal)
    {
        return;
    }

    const int32 NumBones = Skeleton->GetNumBones();
    if (NumBones <= 0)
    {
        return;
    }

    // 1) 부모/길이 테이블
    TArray<int32> ParentIndices;
    TArray<float> BoneLengths;
    ParentIndices.SetNum(NumBones);
    BoneLengths.SetNum(NumBones);

    for (int32 BoneIndex = 0; BoneIndex < NumBones; ++BoneIndex)
    {
        const int32 ParentIndex = Skeleton->GetParentIndex(BoneIndex);
        ParentIndices[BoneIndex] = ParentIndex;

        const FTransform ChildTransform(CurrentSkeletal->GetBoneWorldTransform(BoneIndex));
        FTransform ParentTransform = ChildTransform;

        if (ParentIndex != INDEX_NONE)
        {
            ParentTransform = CurrentSkeletal->GetBoneWorldTransform(ParentIndex);
        }

        const float BoneLength = (ChildTransform.Translation - ParentTransform.Translation).Size();
        BoneLengths[BoneIndex] = BoneLength;
    }

    // 2) Body로 쓸 대표 본 선택 (긴 본 + 루트)
    const float MinLengthToCreateBody = MinColliderLength; // 스케일 보정된 값 사용

    TSet<int32> BodyBoneSet;
    for (int32 BoneIndex = 0; BoneIndex < NumBones; ++BoneIndex)
    {
        const int32 ParentIndex = ParentIndices[BoneIndex];

        if (ParentIndex == INDEX_NONE)
        {
            BodyBoneSet.Add(BoneIndex); // 루트는 무조건 Body
            continue;
        }

        if (BoneLengths[BoneIndex] >= MinLengthToCreateBody)
        {
            BodyBoneSet.Add(BoneIndex); // 충분히 긴 본은 Body
        }
    }

    if (BodyBoneSet.Num() == 0)
    {
        // 최소한 루트 하나는 생성
        BodyBoneSet.Add(0);
    }

    // 3) BodyBone 하나당 그룹 생성
    TMap<int32, int32> BoneToGroupIndex;

    for (int32 BoneIndex = 0; BoneIndex < NumBones; ++BoneIndex)
    {
        if (!BodyBoneSet.Contains(BoneIndex))
        {
            continue;
        }

        const int32 GroupIndex = OutBodyGroups.Emplace();
        FBodyBoneGroup& BodyGroup = OutBodyGroups[GroupIndex];

        BodyGroup.RootBoneIndex = BoneIndex;
        BodyGroup.BoneIndices.Add(BoneIndex);

        BoneToGroupIndex.Add(BoneIndex, GroupIndex);
    }

    // 4) 짧은 본들을 가장 가까운 조상 BodyBone에 머지
    for (int32 BoneIndex = 0; BoneIndex < NumBones; ++BoneIndex)
    {
        if (BodyBoneSet.Contains(BoneIndex))
        {
            continue; // 이미 대표 본
        }

        int32 Ancestor = BoneIndex;
        while (Ancestor != INDEX_NONE && !BodyBoneSet.Contains(Ancestor))
        {
            Ancestor = ParentIndices[Ancestor];
        }

        if (Ancestor != INDEX_NONE)
        {
            if (int32* GroupIndexPtr = BoneToGroupIndex.Find(Ancestor))
            {
                OutBodyGroups[*GroupIndexPtr].BoneIndices.Add(BoneIndex);
            }
        }
        // 조상이 BodyBone이 아니면(루트까지 다 짧은 경우) 과감히 버려도 됨
    }
}

void UPhysicsAsset::CollectGroupBonePoints(const FSkeleton* Skeleton, int32 RootBoneIndex, const TArray<int32>& BoneIndices, TArray<FVector>& OutPointsLocal) const
{
    OutPointsLocal.Empty();

    if (!Skeleton || !CurrentSkeletal)
    {
        return;
    }

    const int32 NumBones = Skeleton->GetNumBones();
    if (RootBoneIndex < 0 || RootBoneIndex >= NumBones)
    {
        return;
    }

    // 루트 본의 월드 트랜스폼 (스케일 제거)
    FTransform RootWorldTransform = CurrentSkeletal->GetBoneWorldTransform(RootBoneIndex);
    RootWorldTransform.Scale3D = FVector(1.0f, 1.0f, 1.0f);

    for (int32 BoneIndex : BoneIndices)
    {
        if (BoneIndex < 0 || BoneIndex >= NumBones)
        {
            continue;
        }

        const int32 ParentIndex = Skeleton->GetParentIndex(BoneIndex);

        FTransform ChildTransform = CurrentSkeletal->GetBoneWorldTransform(BoneIndex);
        ChildTransform.Scale3D = FVector(1.0f, 1.0f, 1.0f);

        FTransform ParentTransform = ChildTransform;
        if (ParentIndex != INDEX_NONE && ParentIndex < NumBones)
        {
            ParentTransform = CurrentSkeletal->GetBoneWorldTransform(ParentIndex);
            ParentTransform.Scale3D = FVector(1.0f, 1.0f, 1.0f);
        }

        // 루트 본 로컬 좌표계로 변환
        FVector LocalStart = InverseTransformPositionNoScale(RootWorldTransform, ParentTransform.Translation);
        FVector LocalEnd = InverseTransformPositionNoScale(RootWorldTransform, ChildTransform.Translation);

        OutPointsLocal.Add(LocalStart);
        OutPointsLocal.Add(LocalEnd);
    }
}

bool UPhysicsAsset::ComputeGroupAxisAndBounds(const TArray<FVector>& PointsLocal, FVector& OutAxis, float& OutMinProjection, float& OutMaxProjection)
{
    const int32 NumPoints = PointsLocal.Num();
    if (NumPoints < 2)
    {
        OutAxis = FVector(0, 0, 1);
        OutMinProjection = 0.0f;
        OutMaxProjection = 0.0f;
        return false;
    }

    // 1) 점들을 이용해 대략적인 축 방향 계산 (짝지어진 Start/End 기준)
    FVector AxisSum = FVector::Zero();
    for (int32 Index = 0; Index + 1 < NumPoints; Index += 2)
    {
        const FVector Direction = PointsLocal[Index + 1] - PointsLocal[Index];
        AxisSum += Direction;
    }

    FVector Axis = AxisSum.GetSafeNormal();
    if (Axis.SizeSquared() < KINDA_SMALL_NUMBER)
    {
        Axis = FVector(0, 0, 1); // 완전 휘어진 경우 fallback
    }

    // 2) 해당 축에 대한 최소/최대 투영값
    float MinProjection = FLT_MAX;
    float MaxProjection = -FLT_MAX;

    for (const FVector& Point : PointsLocal)
    {
        const float Projection = FVector::Dot(Axis, Point);
        MinProjection = FMath::Min(MinProjection, Projection);
        MaxProjection = FMath::Max(MaxProjection, Projection);
    }

    OutAxis = Axis;
    OutMinProjection = MinProjection;
    OutMaxProjection = MaxProjection;
    return true;
}

int32 UPhysicsAsset::FindBodyIndex(FName BodyName) const
{
	if (const int32* Found = BodySetupIndexMap.Find(BodyName))
	{
		return *Found;
	}
	return INDEX_NONE;
}

UBodySetup* UPhysicsAsset::FindBodySetup(FName BodyName) const
{
	const int32 Index = FindBodyIndex(BodyName);
	return (Index != INDEX_NONE) ? BodySetups[Index] : nullptr;
}

int32 UPhysicsAsset::FindConstraintIndex(FName BodyA, FName BodyB) const
{
	for (int32 i = 0; i < Constraints.Num(); ++i)
	{
		const auto& C = Constraints[i];
		if ((C.BodyNameA == BodyA && C.BodyNameB == BodyB) ||
			(C.BodyNameA == BodyB && C.BodyNameB == BodyA))
		{
			return i;
		}
	}
	return INDEX_NONE;
}

TArray<const FPhysicsConstraintSetup*> UPhysicsAsset::GetConstraintsForBody(FName BodyName) const
{
    TArray<const FPhysicsConstraintSetup*> Result;

    for (const FPhysicsConstraintSetup& Constraint : Constraints)
    {
        if (Constraint.BodyNameA == BodyName || Constraint.BodyNameB == BodyName)
        {
            Result.Add(&Constraint);
        }
    }

    return Result;
}

TArray<int32> UPhysicsAsset::GetConstraintIndicesForBody(FName BodyName) const
{
    TArray<int32> Indices;
    
    for (int32 i = 0; i < Constraints.Num(); ++i)
    {
        const auto& C = Constraints[i];
        if (C.BodyNameA == BodyName || C.BodyNameB == BodyName)
        {
            Indices.Add(i);
        }
    }

    return Indices;
}

void UPhysicsAsset::CreateGenerateAllBodySetup(EAggCollisionShapeType ShapeType, FSkeleton* Skeleton, USkeletalMeshComponent* SkeletalComponent)
{
    if (!Skeleton)
    {
        UE_LOG("UPhysicsAsset::CreateGenerateAllBodySetup : Skeleton is null");
        return;
    }

    BodySetups.Empty();
    Constraints.Empty();
    CurrentSkeletal = SkeletalComponent;

    CalculateScaledMinColliderLength();

    // 1) 본 그룹 빌드 (Bone Merge)
    TArray<FBodyBoneGroup> BodyGroups;
    BuildBodyGroups(Skeleton, BodyGroups);

    if (BodyGroups.IsEmpty())
    {
        UE_LOG("UPhysicsAsset::CreateGenerateAllBodySetup : no body groups created");
        return;
    }

    // 2) 각 그룹별 BodySetup 생성
    for (const FBodyBoneGroup& BodyGroup : BodyGroups)
    {
        UBodySetup* BodySetup = NewObject<UBodySetup>();
        if (!BodySetup)
        {
            continue;
        }

        const FName RootBoneName = Skeleton->GetBoneName(BodyGroup.RootBoneIndex);
        BodySetup->BoneName = RootBoneName;

        switch (ShapeType)
        {
            case EAggCollisionShapeType::Capsule:
                BodySetup->AddCapsule(FitCapsuleToBoneGroup(Skeleton, BodyGroup));
                break;

            case EAggCollisionShapeType::Box:
                BodySetup->AddBox(FitBoxToBoneGroup(Skeleton, BodyGroup));
                break;

            case EAggCollisionShapeType::Sphere:
                BodySetup->AddSphere(FitSphereToBoneGroup(Skeleton, BodyGroup));
                break;

            default:
                BodySetup->AddCapsule(FitCapsuleToBoneGroup(Skeleton, BodyGroup));
                break;
        }

        BodySetups.Add(BodySetup);
    }

    // 3) Constraints는 대표 본들만 대상으로 생성
    TArray<int32> BodyBoneIndices;
    BodyBoneIndices.Reserve(BodyGroups.Num());
    for (const FBodyBoneGroup& BodyGroup : BodyGroups)
    {
        BodyBoneIndices.Add(BodyGroup.RootBoneIndex);
    }

    GenerateConstraintsFromSkeleton(Skeleton, BodyBoneIndices, SkeletalComponent);
    BuildRuntimeCache();
}

void UPhysicsAsset::SelectBonesForBodies(const FSkeleton* Skeleton, TArray<int32>& OutBones) const
{
    OutBones.Empty();
    if (!Skeleton)
    {
        return;
    }

    const int32 NumBones = Skeleton->GetNumBones();
    const float MinBoneLength = 0.1f;

    for (int32 BoneIndex = 0; BoneIndex < NumBones; ++BoneIndex)
    {
        const int32 ParentIndex = Skeleton->GetParentIndex(BoneIndex);
        if (ParentIndex == INDEX_NONE)
        {
            OutBones.Add(BoneIndex);
            continue;
        }

        const FTransform ParentTransform(CurrentSkeletal->GetBoneWorldTransform(ParentIndex));
        const FTransform ChildTransform(CurrentSkeletal->GetBoneWorldTransform(BoneIndex));

        const float Length = (ChildTransform.Translation - ParentTransform.Translation).Size();
        
        if (Length >= MinBoneLength)
        {
            OutBones.Add(BoneIndex);
        }
    }
}

FBonePoints UPhysicsAsset::GetBonePoints(const FSkeleton* Skeleton, int32 BoneIndex) const
{
    FBonePoints Points{};

    if (!Skeleton)
    {
        return Points;
    }

    const int32 NumBones = Skeleton->GetNumBones();
    if (BoneIndex < 0 || BoneIndex >= NumBones)
    {
        return Points;
    }

    const int32 ParentIndex = Skeleton->GetParentIndex(BoneIndex);
    const FTransform ChildT = CurrentSkeletal->GetBoneWorldTransform(BoneIndex);
    FTransform ParentT = ChildT;
    if (ParentIndex != INDEX_NONE && ParentIndex < NumBones)
    {
        ParentT = CurrentSkeletal->GetBoneWorldTransform(ParentIndex);
    }

    Points.Start = ParentT.Translation;
    Points.End = ChildT.Translation;

    return Points;
}

FKCapsuleElem UPhysicsAsset::FitCapsuleToBoneGroup(const FSkeleton* Skeleton, const FBodyBoneGroup& BodyGroup)
{
    FKCapsuleElem CapsuleElement;

    if (!Skeleton || BodyGroup.RootBoneIndex == INDEX_NONE)
    {
        CapsuleElement.Radius = 0.1f;
        CapsuleElement.HalfLength = 0.0f;
        CapsuleElement.Center = FVector::Zero();
        CapsuleElement.Rotation = FQuat::Identity();
        return CapsuleElement;
    }

    TArray<FVector> PointsLocal;
    CollectGroupBonePoints(Skeleton, BodyGroup.RootBoneIndex, BodyGroup.BoneIndices, PointsLocal);

    if (PointsLocal.Num() < 2)
    {
        return FitCapsuleToBone(Skeleton, BodyGroup.RootBoneIndex);
    }

    FVector Axis;
    float MinProjection = 0.0f;
    float MaxProjection = 0.0f;
    ComputeGroupAxisAndBounds(PointsLocal, Axis, MinProjection, MaxProjection);

    const float ChainLength = MaxProjection - MinProjection;
    const float ChainHalfLength = ChainLength * 0.5f;
    const float MidProjection = (MinProjection + MaxProjection) * 0.5f;

    // 캡슐 중심 (루트 본 로컬)
    CapsuleElement.Center = Axis * MidProjection;

    // 캡슐 축을 로컬 Z축과 맞추기 위한 회전
    const FVector CapsuleLocalAxis = FVector(0, 0, 1);
    CapsuleElement.Rotation = FQuat::FindBetweenVectors(CapsuleLocalAxis, Axis);

    // 길이/반지름 설정 (적당히 통통하게)
    CapsuleElement.HalfLength = ChainHalfLength * 0.5f;      // 체인 길이의 1/2 를 HalfLength로
    CapsuleElement.Radius = ChainLength * 0.25f;       // 체인 길이의 1/4를 반지름으로 (튜닝 가능)

    return CapsuleElement;
}

FKBoxElem UPhysicsAsset::FitBoxToBoneGroup(const FSkeleton* Skeleton, const FBodyBoneGroup& BodyGroup)
{
    FKBoxElem BoxElement;

    if (!Skeleton || BodyGroup.RootBoneIndex == INDEX_NONE)
    {
        BoxElement.Extents = FVector(0.1f, 0.1f, 0.1f);
        BoxElement.Center = FVector::Zero();
        BoxElement.Rotation = FQuat::Identity();
        return BoxElement;
    }

    TArray<FVector> PointsLocal;
    CollectGroupBonePoints(Skeleton, BodyGroup.RootBoneIndex, BodyGroup.BoneIndices, PointsLocal);

    if (PointsLocal.Num() < 2)
    {
        return FitBoxToBone(Skeleton, BodyGroup.RootBoneIndex);
    }

    FVector Axis;
    float MinProjection = 0.0f;
    float MaxProjection = 0.0f;
    ComputeGroupAxisAndBounds(PointsLocal, Axis, MinProjection, MaxProjection);

    const float ChainLength = MaxProjection - MinProjection;
    const float ChainHalfLength = ChainLength * 0.5f;
    const float MidProjection = (MinProjection + MaxProjection) * 0.5f;

    // 박스 중심 (루트 본 로컬)
    BoxElement.Center = Axis * MidProjection;

    // 본 로컬 Z축 기준
    const FVector BoxLocalZ = FVector(0, 0, 1);
    BoxElement.Rotation = FQuat::FindBetweenVectors(BoxLocalZ, Axis);

    const float HalfDepth = ChainHalfLength;
    const float HalfWidth = ChainHalfLength * 0.4f;

    BoxElement.Extents = FVector(HalfWidth, HalfWidth, HalfDepth);
    return BoxElement;
}

FKSphereElem UPhysicsAsset::FitSphereToBoneGroup(const FSkeleton* Skeleton, const FBodyBoneGroup& BodyGroup)
{
    FKSphereElem SphereElement;

    if (!Skeleton || BodyGroup.RootBoneIndex == INDEX_NONE)
    {
        SphereElement.Radius = 0.1f;
        SphereElement.Center = FVector::Zero();
        return SphereElement;
    }

    TArray<FVector> PointsLocal;
    CollectGroupBonePoints(Skeleton, BodyGroup.RootBoneIndex, BodyGroup.BoneIndices, PointsLocal);

    if (PointsLocal.Num() < 2)
    {
        return FitSphereToBone(Skeleton, BodyGroup.RootBoneIndex);
    }

    // 평균 중심
    FVector Center = FVector::Zero();
    for (const FVector& Point : PointsLocal)
    {
        Center += Point;
    }
    Center /= static_cast<float>(PointsLocal.Num());

    float MaxRadiusSquared = 0.0f;
    for (const FVector& Point : PointsLocal)
    {
        const float DistanceSquared = (Point - Center).SizeSquared();
        MaxRadiusSquared = FMath::Max(MaxRadiusSquared, DistanceSquared);
    }

    SphereElement.Center = Center;
    SphereElement.Radius = FMath::Sqrt(MaxRadiusSquared) * 1.1f; // 살짝 여유

    return SphereElement;
}

FKSphereElem UPhysicsAsset::FitSphereToBone(const FSkeleton* Skeleton, int32 BoneIndex)
{
    FBonePoints Point = GetBonePoints(Skeleton, BoneIndex);
    const FVector BoneDir = (Point.End - Point.Start);
    const float BoneLen = BoneDir.Size();

    FKSphereElem Elem;

    if (BoneLen < KINDA_SMALL_NUMBER)
    {
        Elem.Radius = 0.1f;
        Elem.Center = FVector::Zero();
    }
    else
    {
        Elem.Radius = BoneLen * 0.5f;

        // 본 로컬 Z축 방향으로 중심 오프셋 (애니메이션 포즈와 무관)
        Elem.Center = FVector(0, 0, BoneLen * 0.5f);
    }

    return Elem;
}

FKBoxElem UPhysicsAsset::FitBoxToBone(const FSkeleton* Skeleton, int32 BoneIndex)
{
    FBonePoints Point = GetBonePoints(Skeleton, BoneIndex);

    const FVector BoneDir = (Point.End - Point.Start);
    const float BoneLen = BoneDir.Size();

    FKBoxElem Elem;

    if (BoneLen < KINDA_SMALL_NUMBER)
    {
        Elem.Extents = FVector(0.1f, 0.1f, 0.1f);
        Elem.Center = FVector::Zero();
        Elem.Rotation = FQuat::Identity();
    }
    else
    {
        const float HalfDepth = BoneLen * 0.5f; // Main axis along bone length
        const float HalfWidth = BoneLen * 0.15f; // Other axes smaller

        Elem.Extents = FVector(HalfWidth, HalfWidth, HalfDepth);

        // 본 로컬 Z축 방향으로 중심 오프셋 (애니메이션 포즈와 무관)
        Elem.Center = FVector(0, 0, BoneLen * 0.5f);
        // 본 로컬 축에 정렬 (회전 없음)
        Elem.Rotation = FQuat::Identity();
    }

    return Elem;
}

FKCapsuleElem UPhysicsAsset::FitCapsuleToBone(const FSkeleton* Skeleton, int32 BoneIndex)
{
    FBonePoints Point = GetBonePoints(Skeleton, BoneIndex);

    const FVector BoneDir = (Point.End - Point.Start);
    const float BoneLen = BoneDir.Size();

    FKCapsuleElem Elem;

    if (BoneLen < KINDA_SMALL_NUMBER)
    {
        Elem.Radius = 0.1f;
        Elem.HalfLength = 0.0f;
        Elem.Center = FVector::Zero();
        Elem.Rotation = FQuat::Identity();
    }
    else
    {
        Elem.Radius = BoneLen / 4.0f;
        Elem.HalfLength = BoneLen / 4.0f;

        // 본 로컬 Z축 방향으로 중심 오프셋 (애니메이션 포즈와 무관)
        Elem.Center = FVector(0, 0, BoneLen * 0.5f);
        // 본 로컬 축에 정렬 (회전 없음)
        Elem.Rotation = FQuat::Identity();
    }

    return Elem;
}

void UPhysicsAsset::GenerateConstraintsFromSkeleton(const FSkeleton* Skeleton, const TArray<int32>& BoneIndicesToCreate, USkeletalMeshComponent* SkeletalComponent)
{
    if (!Skeleton)
    {
        return;
    }

    TSet<int32> BodyBones;
    for (int32 BoneIndex : BoneIndicesToCreate)
    {
        BodyBones.Add(BoneIndex);
    }

    for (int32 BoneIndex : BoneIndicesToCreate)
    {
        const int32 ParentIndex = Skeleton->GetParentIndex(BoneIndex);
        if (ParentIndex == INDEX_NONE)
        {
            continue;
        }

        if (!BodyBones.Contains(ParentIndex))
        {
            continue;
        }

        const FName ChildBoneName = Skeleton->GetBoneName(BoneIndex);
        const FName ParentBoneName = Skeleton->GetBoneName(ParentIndex);

        FPhysicsConstraintSetup Setup{};
        Setup.BodyNameA = ParentBoneName;
        Setup.BodyNameB = ChildBoneName;

        FTransform ParentWorldTransform;
        FTransform ChildWorldTransform;
        if (SkeletalComponent)
        {
            ParentWorldTransform = SkeletalComponent->GetBoneWorldTransform(ParentIndex);
            ChildWorldTransform = SkeletalComponent->GetBoneWorldTransform(BoneIndex);

            ParentWorldTransform.Scale3D = FVector(1.0f, 1.0f, 1.0f);
            ChildWorldTransform.Scale3D = FVector(1.0f, 1.0f, 1.0f);
        }
        else
        {
            ParentWorldTransform = SkeletalComponent->GetBoneWorldTransform(ParentIndex);
            ChildWorldTransform = SkeletalComponent->GetBoneWorldTransform(BoneIndex);

            ParentWorldTransform.Scale3D = FVector(1.0f, 1.0f, 1.0f);
            ChildWorldTransform.Scale3D = FVector(1.0f, 1.0f, 1.0f);
        }

        // 조인트 프레임 설정
        // PxD6Joint 축 정의: X축 = Twist, Y축 = Swing1(Twist 0도 기준), Z축 = Swing2
        //
        // 목표:
        // - 조인트 X축(Twist 회전축) = 본의 Z축 (본 길이 방향)
        // - 조인트 Y축(Twist 0도 기준) = 본의 X축
        // - 조인트 Z축 = 본의 -Y축
        //
        // 변환 과정:
        // 1) ZToX: Y축 기준 -90도 → 본 Z축을 조인트 X축으로 맞춤
        // 2) TwistRef: X축 기준 -90도 → 조인트 Y축을 본 X축 방향으로 (Twist 0도 기준을 +X로)
        FQuat ZToX = FQuat::FromAxisAngle(FVector(0, 1, 0), -XM_PIDIV2);
        FQuat TwistRef = FQuat::FromAxisAngle(FVector(1, 0, 0), -XM_PIDIV2);
        FQuat JointFrameRotation = ZToX * TwistRef;

        // LocalFrameA: Parent Body 기준, Child 위치에서 축 보정
        FTransform RelativeTransform = ParentWorldTransform.GetRelativeTransform(ChildWorldTransform);
        Setup.LocalFrameA = FTransform(RelativeTransform.Translation, RelativeTransform.Rotation * JointFrameRotation, FVector(1, 1, 1));

        // LocalFrameB: Child Body 기준, 원점에서 축 보정
        Setup.LocalFrameB = FTransform(FVector::Zero(), JointFrameRotation, FVector(1, 1, 1));

        Setup.TwistLimitMin = DegreesToRadians(-45.0f);
        Setup.TwistLimitMax = DegreesToRadians(45.0f);
        Setup.SwingLimitY = DegreesToRadians(30.0f);
        Setup.SwingLimitZ = DegreesToRadians(30.0f);

        Constraints.Add(Setup);
    }
}

float UPhysicsAsset::GetMeshScale() const
{
    if (!CurrentSkeletal)
    {
        return 1.0f;
    }

    // Root bone의 월드 스케일 가져오기
    const FTransform RootTransform = CurrentSkeletal->GetBoneWorldTransform(0);
    const FVector Scale = RootTransform.Scale3D;

    return FMath::Max(FMath::Abs(Scale.X), FMath::Abs(Scale.Y), FMath::Abs(Scale.Z));
}

void UPhysicsAsset::CalculateScaledMinColliderLength()
{
    float MeshScale = GetMeshScale();
    // MinColliderLength *= MeshScale;
}

bool UPhysicsAsset::Load(const FString& InFilePath, ID3D11Device* InDevice)
{
    JSON Root;
    if (!FJsonSerializer::LoadJsonFromFile(Root, UTF8ToWide(InFilePath)))
    {
        UE_LOG("[UPhysicsAsset] Failed to load file: %s", InFilePath.c_str());
        return false;
    }

    Serialize(true, Root);
    SetFilePath(NormalizePath(InFilePath)); 
    
    BuildRuntimeCache();

    UE_LOG("[UPhysicsAsset] Loaded successfully: %s", InFilePath.c_str());
    return true;
}

bool UPhysicsAsset::SaveToFile(const FString& FilePath)
{
    JSON JsonData = JSON::Make(JSON::Class::Object);

    Serialize(false, JsonData);

    std::ofstream OutFile(FilePath, std::ios::out);
    if (!OutFile.is_open())
    {
        UE_LOG("Failed to open file for writing: %s", FilePath.c_str());
        return false;
    }

    OutFile << JsonData.dump(4); // Pretty print
    OutFile.close();

    UE_LOG("UPhysicsAsset saved to: %s", FilePath.c_str());
    return true;
}

void UPhysicsAsset::Serialize(const bool bInIsLoading, JSON& InOutHandle)
{
    Super::Serialize(bInIsLoading, InOutHandle);

    if (bInIsLoading)
    {
        // =========================================================
        // [LOAD] 역직렬화
        // =========================================================
        if (!InOutHandle.hasKey("Type") || InOutHandle["Type"].ToString() != "PhysicsAsset") { return; }

        // 기존 데이터 초기화 (BodySetups)
        for (UBodySetup* BodySetup : BodySetups)
        {
            if (BodySetup) ObjectFactory::DeleteObject(BodySetup);
        }
        BodySetups.Empty();

        // 기본 정보 로드
        FString NameStr;
        if (FJsonSerializer::ReadString(InOutHandle, "Name", NameStr))
        {
            Name = FName(NameStr);
        }

        // BodySetups 배열 로드
        if (InOutHandle.hasKey("BodySetups"))
        {
            JSON BodySetupsArray = InOutHandle["BodySetups"];
            for (int32 i = 0; i < BodySetupsArray.size(); ++i)
            {
                JSON BodySetupJson = BodySetupsArray[i];

                // BodySetup 생성
                UBodySetup* NewBodySetup = Cast<UBodySetup>(NewObject(UBodySetup::StaticClass()));
                if (NewBodySetup)
                {
                    NewBodySetup->Serialize(true, BodySetupJson);
                    BodySetups.Add(NewBodySetup);
                }
            }
        }

        // Constraints 배열 로드
        Constraints.Empty();
        if (InOutHandle.hasKey("Constraints"))
        {
            JSON& ConstraintArray = InOutHandle["Constraints"];
            for (int32 i = 0; i < ConstraintArray.size(); ++i)
            {
                JSON& Item = ConstraintArray.at(i);

                FPhysicsConstraintSetup Setup{};
                Setup.Serialize(true, Item);
                Constraints.Add(Setup);
            }
        }

        // 런타임 캐시 구축 (Bounds 계산 등)
        BuildRuntimeCache();
    }
    else
    {
        // =========================================================
        // [SAVE] 직렬화
        // =========================================================
        InOutHandle["Type"] = "PhysicsAsset";
        InOutHandle["Name"] = Name.ToString();

        // Physics 배열 저장
        JSON PhysicsArray = JSON::Make(JSON::Class::Array);
        for (UBodySetup* BodySetup : BodySetups)
        {
            if (!BodySetup) continue;

            JSON BodySetupJson = JSON::Make(JSON::Class::Object);
            BodySetup->Serialize(false, BodySetupJson);

            PhysicsArray.append(BodySetupJson);
        }
        InOutHandle["BodySetups"] = PhysicsArray;

        JSON ConstraintArray = JSON::Make(JSON::Class::Array); 
        for (FPhysicsConstraintSetup& Setup : Constraints)
        { 
            if (!Setup.BodyNameA.IsValid() || !Setup.BodyNameB.IsValid())
            {
                continue;
            }

            JSON ConstraintJson = JSON::Make(JSON::Class::Object);
            Setup.Serialize(false, ConstraintJson);
            ConstraintArray.append(ConstraintJson);
        } 
        InOutHandle["Constraints"] = ConstraintArray;
    }
}

void FKAggregateGeom::Clear()
{
    SphereElements.Empty();
    BoxElements.Empty();
    CapsuleElements.Empty();
    ConvexElements.Empty();
}

void FPhysicsConstraintSetup::Serialize(const bool bInIsLoading,  JSON& InOutHandle)
{
    if (bInIsLoading)
    {
        FString BodyAName;
        if (FJsonSerializer::ReadString(InOutHandle, "BodyA", BodyAName))
        {
            BodyNameA = FName(BodyAName);
        }
        FString BodyBName;
        if (FJsonSerializer::ReadString(InOutHandle, "BodyB", BodyBName))
        {
            BodyNameB = FName(BodyBName);
        }

        FJsonSerializer::ReadTransform(InOutHandle, "LocalFrameA", LocalFrameA);
        FJsonSerializer::ReadTransform(InOutHandle, "LocalFrameB", LocalFrameB);
        FJsonSerializer::ReadFloat(InOutHandle, "TwistMin", TwistLimitMin);
        FJsonSerializer::ReadFloat(InOutHandle, "TwistMax", TwistLimitMax);
        FJsonSerializer::ReadFloat(InOutHandle, "SwingY", SwingLimitY);
        FJsonSerializer::ReadFloat(InOutHandle, "SwingZ", SwingLimitZ);
    }
    else
    {
        InOutHandle["BodyA"] = BodyNameA.ToString().c_str();
        InOutHandle["BodyB"] = BodyNameB.ToString().c_str();
        InOutHandle["LocalFrameA"] = FJsonSerializer::TransformToJson(LocalFrameA);
        InOutHandle["LocalFrameB"] = FJsonSerializer::TransformToJson(LocalFrameB);
        InOutHandle["TwistMin"] = TwistLimitMin;
        InOutHandle["TwistMax"] = TwistLimitMax;
        InOutHandle["SwingY"] = SwingLimitY;
        InOutHandle["SwingZ"] = SwingLimitZ;
    }
}