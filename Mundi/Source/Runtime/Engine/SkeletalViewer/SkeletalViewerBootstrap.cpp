#include "pch.h"
#include "SkeletalViewerBootstrap.h"
#include "CameraActor.h"
#include "Source/Runtime/Engine/SkeletalViewer/ViewerState.h"
#include "FViewport.h"
#include "FSkeletalViewerViewportClient.h"
#include "Source/Runtime/Engine/GameFramework/SkeletalMeshActor.h"
#include "Source/Runtime/Engine/GameFramework/StaticMeshActor.h"
#include "Source/Runtime/Engine/Components/StaticMeshComponent.h"
#include "Source/Runtime/Engine/Physics/PhysScene.h"

ViewerState* SkeletalViewerBootstrap::CreateViewerState(const char* Name, UWorld* InWorld, ID3D11Device* InDevice)
{
    if (!InDevice) return nullptr;

    ViewerState* State = new ViewerState();
    State->Name = Name ? Name : "Viewer";

    // Preview world 만들기
    State->World = NewObject<UWorld>();
    State->World->SetWorldType(EWorldType::PreviewMinimal);  // Set as preview world for memory optimization
    State->World->Initialize();
    State->World->GetRenderSettings().DisableShowFlag(EEngineShowFlags::SF_EditorIcon);

    // 뷰어에서 래그돌 물리 시뮬레이션을 위해 PhysScene 초기화
    State->World->InitializePhysScene();

    State->World->GetGizmoActor()->SetSpace(EGizmoSpace::Local);
    
    // Viewport + client per tab
    State->Viewport = new FViewport();
    // 프레임 마다 initial size가 바꿜 것이다
    State->Viewport->Initialize(0, 0, 1, 1, InDevice);

    auto* Client = new FSkeletalViewerViewportClient();
    Client->SetWorld(State->World);
    Client->SetViewportType(EViewportType::Perspective);
    Client->SetViewMode(EViewMode::VMI_Lit_Phong);
    Client->GetCamera()->SetActorLocation(FVector(3, 0, 2));

    State->Client = Client;
    State->Viewport->SetViewportClient(Client);

    State->World->SetEditorCameraActor(Client->GetCamera());

    // Spawn a persistent preview actor (mesh can be set later from UI)
    if (State->World)
    {
        ASkeletalMeshActor* Preview = State->World->SpawnActor<ASkeletalMeshActor>();
        State->PreviewActor = Preview;

        // 뷰어(에디터 모드)에서도 TickComponent가 호출되도록 설정
        if (USkeletalMeshComponent* SkelComp = Preview->GetSkeletalMeshComponent())
        {
            SkelComp->bTickInEditor = true;
        }

        // 바닥 스태틱 메시 생성 (래그돌이 떨어지지 않도록)
        FTransform FloorTransform;
        FloorTransform.Translation = FVector(0, 0, -0.5f);  // 상단 면이 Z=0에 오도록
        FloorTransform.Scale3D = FVector(50, 50, 1);

        AStaticMeshActor* FloorActor = State->World->SpawnActor<AStaticMeshActor>(FloorTransform);
        if (FloorActor)
        {
            if (UStaticMeshComponent* FloorMeshComp = FloorActor->GetStaticMeshComponent())
            {
                FloorMeshComp->SetStaticMesh("Data/cube-tex.obj");
                FloorMeshComp->SetEnableCollision(true);
                FloorMeshComp->SetPhysMaterialPreset(2);  // 2 = Wood

                // 뷰어 월드에서는 BeginPlay가 호출되지 않으므로 직접 물리 초기화
                if (FPhysScene* PhysScene = State->World->GetPhysScene())
                {
                    FloorMeshComp->InitPhysics(*PhysScene);
                }
            }
        }
    }

    if (InWorld)
    {
        State->World->GetRenderSettings().SetShowFlags(InWorld->GetRenderSettings().GetShowFlags());
        State->World->GetRenderSettings().DisableShowFlag(EEngineShowFlags::SF_EditorIcon);;
    }

    return State;
}

void SkeletalViewerBootstrap::DestroyViewerState(ViewerState*& State)
{
    if (!State) return;
    if (State->Viewport) { delete State->Viewport; State->Viewport = nullptr; }
    if (State->Client) { delete State->Client; State->Client = nullptr; }
    if (State->World) { ObjectFactory::DeleteObject(State->World); State->World = nullptr; }
    delete State; State = nullptr;
}
