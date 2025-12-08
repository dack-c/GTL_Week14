#include "pch.h"
#include "TriggerComponent.h"
#include "LuaScriptComponent.h"
#include "BillboardComponent.h"

FVector UTriggerComponent::CharacterPos = FVector(1000000, 1000000, 10000000); //하드코딩 렛츠고

UTriggerComponent::UTriggerComponent()
{
	bCanEverTick = true;
}
UTriggerComponent::~UTriggerComponent()
{

}
void UTriggerComponent::OnRegister(UWorld* InWorld)
{
	Super::OnRegister(InWorld);
	if (!std::strcmp(this->GetClass()->Name, UTriggerComponent::StaticClass()->Name) && !SpriteComponent && !InWorld->bPie)
	{
		CREATE_EDITOR_COMPONENT(SpriteComponent, UBillboardComponent);
		SpriteComponent->SetTexture(GDataDir + "/UI/Icons/EmptyActor.dds");
	}
}

void UTriggerComponent::TickComponent(float DeltaTime)
{
	//매 틱 CharacterActor의 위치를 가져와서 체크
	FMatrix WorldMatrix = GetWorldMatrix();
	FMatrix InvWorld = WorldMatrix.Inverse();
	FVector CharacterInTriggerLocal = CharacterPos * InvWorld;
	if (abs(CharacterInTriggerLocal.X) <= BoxExtent.X && abs(CharacterInTriggerLocal.Y) <= BoxExtent.Y && abs(CharacterInTriggerLocal.Z) <= BoxExtent.Z)
	{
		auto Components = Owner->GetOwnedComponents();
		for (UActorComponent* ActorComp : Components)
		{
			if (ULuaScriptComponent* Lua = Cast<ULuaScriptComponent>(ActorComp))
			{
				Lua->CallFunction(LuaFunctionName.c_str());
			}
		}
	}
}


void UTriggerComponent::Serialize(const bool bInIsLoading, JSON& InOutHandle)
{
	Super::Serialize(bInIsLoading, InOutHandle);
}

void UTriggerComponent::DuplicateSubObjects()
{
	Super::DuplicateSubObjects();
}
void UTriggerComponent::RenderDebugVolume(URenderer* Renderer) const
{
	// visible = 에디터용
	// hiddeningame = 파이용

	const FVector Extent = BoxExtent;
	const FTransform WorldTransform = GetWorldTransform();

	TArray<FVector> StartPoints;
	TArray<FVector> EndPoints;
	TArray<FVector4> Colors;

	FVector local[8] = {
		{-Extent.X, -Extent.Y, -Extent.Z}, {+Extent.X, -Extent.Y, -Extent.Z},
		{-Extent.X, +Extent.Y, -Extent.Z}, {+Extent.X, +Extent.Y, -Extent.Z},
		{-Extent.X, -Extent.Y, +Extent.Z}, {+Extent.X, -Extent.Y, +Extent.Z},
		{-Extent.X, +Extent.Y, +Extent.Z}, {+Extent.X, +Extent.Y, +Extent.Z},
	};

	//월드 space로 변환
	FVector WorldSpace[8];
	for (int i = 0; i < 8; i++)
	{
		WorldSpace[i] = WorldTransform.TransformPosition(local[i]);
	}

	static const int Edge[12][2] = {
		{0,1},{1,3},{3,2},{2,0}, // bottom
		{4,5},{5,7},{7,6},{6,4}, // top
		{0,4},{1,5},{2,6},{3,7}  // verticals
	};
	for (int i = 0; i < 12; ++i)
	{
		StartPoints.Add(WorldSpace[Edge[i][0]]);
		EndPoints.Add(WorldSpace[Edge[i][1]]);
		Colors.Add(DebugVolumeColor); // 동일 색으로 라인 렌더
	}

	Renderer->AddLines(StartPoints, EndPoints, Colors);
}
