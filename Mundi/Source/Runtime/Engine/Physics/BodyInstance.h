#pragma once


class UPrimitiveComponent;
class UBodySetup;
struct FPhysicsWorld;




/**
 * @brief 엔진에서 "물리 바디 한 개"를 대표하는 런타임 객체, 내부에 PxRigidDynamic / PxRigidStatic을 들고있고 OwnerComponent 포인터도 들고있다.
 */

struct FBodyInstance
{
    void InitDynamic(FPhysicsWorld& World, const FTransform& WorldTransform, float Mass);
    void InitStatic(FPhysicsWorld& World, const FTransform& WorldTransform);

    void Terminate(FPhysicsWorld& World);

    void AddForce(const FVector& Force);
    FTransform GetWorldTransform() const;


public:
    UPrimitiveComponent* OwnerComponent = nullptr;
    UBodySetup*          BodySetup      = nullptr;
    PxRigidActor*        RigidActor     = nullptr;
};
