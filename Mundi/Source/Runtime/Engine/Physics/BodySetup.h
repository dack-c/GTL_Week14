#include "BodySetupCore.h"
#include "FKShapeElem.h"

struct FKSphereElem;
struct FKBoxElem;
struct FKCapsuleElem;
struct FKConvexElem;
struct FKTriangleMeshElem;

struct FKAggregateGeom
{
    TArray<FKSphereElem> SphereElements;
    TArray<FKBoxElem>    BoxElements;
    TArray<FKCapsuleElem>  CapsuleElements;
    TArray<FKConvexElem> ConvexElements;
    TArray<FKTriangleMeshElem> TriangleMeshElements;

    void Clear();
    void Serialize(const bool bInIsLoading, JSON& InOutHandle);
};

class UPhysicalMaterial;
class UBodySetup : public UBodySetupCore
{
    DECLARE_CLASS(UBodySetup, UBodySetupCore)
public:
    UBodySetup();
	~UBodySetup() override;

    FKAggregateGeom AggGeom;             // 이 Body가 가진 Primitive Collision 모음

    // ===============================================
    // Collision Setting
    // ===============================================
    float Mass = 10.0f;                
    float LinearDamping = 1.0f;  // 위치의 시간 당 감쇠량
    float AngularDamping = 1.0f; // 회전의 시간 당 감쇠량

    UPhysicalMaterial* PhysMaterial = nullptr;

    // ===============================================
    // Physics 적용 Setting
    // ===============================================
    ECollisionState CollisionState = ECollisionState::QueryAndPhysics; // 충돌 설정

    bool bSimulatePhysics = true; // false일 시 Static
    bool bEnableGravity = true;   // 위가 false면 의미없음

    bool bCachedDataDirty = true;

    void AddSphere(const FKSphereElem& Elem);
    void AddBox(const FKBoxElem& Elem);
    void AddCapsule(const FKCapsuleElem& Elem);
    void AddTriangleMesh(const FKTriangleMeshElem& Elem);

    void BuildCachedData();
    void Serialize(const bool bInIsLoading, JSON& InOutHandle) override;

    // ===============================================
    // 유틸리티
    // ===============================================
	uint32 GetTotalShapeCount() const;
};