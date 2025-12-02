#pragma once
#include "sol/sol.hpp"
#include "Vector.h"

class AActor;

struct LuaContactInfo
{
    AActor* OtherActor = nullptr;
    FVector Position;
    FVector Normal;
    FVector Impulse;
};

struct LuaTriggerInfo
{
    AActor* OtherActor = nullptr;
    bool bIsEnter = false;
};