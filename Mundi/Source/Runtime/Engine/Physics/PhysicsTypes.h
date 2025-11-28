#pragma once

#include <PxPhysicsAPI.h>
#include <DirectXMath.h>

using namespace DirectX;   // 이건 그나마 괜찮

// FVector -> PxVec3
inline physx::PxVec3 ToPx(const FVector& v)
{
    return physx::PxVec3(v.X, v.Y, v.Z); // 지금은 엔진과 축 동일하게 사용
}

// PxVec3 -> FVector
inline FVector FromPx(const physx::PxVec3& v)
{
    return FVector(v.x, v.y, v.z);
}

// FQuat -> PxQuat
inline physx::PxQuat ToPx(const FQuat& q)
{
    return physx::PxQuat(q.X, q.Y, q.Z, q.W);
}

inline FQuat FromPx(const physx::PxQuat& q)
{
    return FQuat(q.x, q.y, q.z, q.w);
}

// FTransform -> PxTransform
inline physx::PxTransform ToPx(const FTransform& t)
{
    return physx::PxTransform(
        ToPx(t.Translation),
        ToPx(t.Rotation)
    );
}

inline FTransform FromPx(const physx::PxTransform& t)
{
    return FTransform(FromPx(t.p), FromPx(t.q), FVector(1, 1, 1));
}