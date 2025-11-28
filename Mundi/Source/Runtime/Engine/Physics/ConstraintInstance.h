#pragma once
#include "BodyInstance.h"



struct FConstraintLimitData;

struct FConstraintInstance
{

public:
    void InitD6(FPhysicsWorld& World, const FTransform& ParentFrame, const FTransform& ChildFrame, const FConstraintLimitData& Limits);

    void Terminate(FPhysicsWorld& World);
    
public:
    FBodyInstance* ParentBody   = nullptr;
    FBodyInstance* ChildBody    = nullptr;
    PxJoint*       Joint        = nullptr;
};
