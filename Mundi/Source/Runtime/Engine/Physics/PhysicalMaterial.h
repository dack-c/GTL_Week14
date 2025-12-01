#pragma once
class UPhysicalMaterial : public UObject
{
    DECLARE_CLASS(UPhysicalMaterial, UObject)
public:
    float StaticFriction;   // 물체 정지 상태일 시, 움직임 시작하는 걸 방해하는 마찰 계수
    float DynamicFriction;  // 물체 운동 상태일 시, 움직임을 지속하는 걸 방해하는 마찰 계수
    float Restitution;      // 탄성, 0 : 완전 비탄성 1 : 완전 탄성
    float Density;          // BodySetup::Mass = BodySetup::AggGeom * Density

    UPhysicalMaterial()
        : StaticFriction(0.5f)
        , DynamicFriction(0.5f)
        , Restitution(0.5f)
        , Density(1.0f)
    {
	}
    
    void Serialize(const bool bInIsLoading, JSON& InOutHandle)
    {
        Super::Serialize(bInIsLoading, InOutHandle);

        if (bInIsLoading)
        {
            FJsonSerializer::ReadFloat(InOutHandle, "StaticFriction", StaticFriction);
            FJsonSerializer::ReadFloat(InOutHandle, "DynamicFriction", DynamicFriction);
            FJsonSerializer::ReadFloat(InOutHandle, "Restitution", Restitution);
            FJsonSerializer::ReadFloat(InOutHandle, "Density", Density);
        }
        else
        {
            InOutHandle["StaticFriction"] = StaticFriction;
            InOutHandle["DynamicFriction"] = DynamicFriction;
            InOutHandle["Restitution"] = Restitution;
            InOutHandle["Density"] = Density;
        }
    }
};