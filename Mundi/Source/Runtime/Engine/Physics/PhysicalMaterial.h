#pragma once
class UPhysicalMaterial : public UObject
{
    DECLARE_CLASS(UPhysicalMaterial, UObject)
public:
    float StaticFriction;
    float DynamicFriction;
    float Restitution;
    float Density;
};