#include "pch.h"
#include "PlayerStart.h"
#include "BillboardComponent.h"

APlayerStart::APlayerStart()
	: PlayerStartTag(FName())
	, bIsPIEPlayerStart(false)
	, EditorIcon(nullptr)
{
	ObjectName = "PlayerStart";

	EditorIcon = CreateDefaultSubobject<UBillboardComponent>("EditorIcon");
	RootComponent = EditorIcon;

	if (EditorIcon)
	{
		EditorIcon->SetTexture(GDataDir + "/Icon/PlayerStart_64.dds");
		EditorIcon->SetHiddenInGame(true);
	}
}
