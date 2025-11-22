#include "pch.h"
#include "ParticleViewerWindow.h"
#include "Source/Runtime/Renderer/FViewport.h"
#include "Source/Runtime/Renderer/FViewportClient.h"
#include "Source/Runtime/Engine/Particle/ParticleSystem.h"
#include "Source/Runtime/Engine/Particle/ParticleEmitter.h"
#include "Source/Runtime/Engine/Particle/ParticleLODLevel.h"
#include "Source/Runtime/Engine/Particle/Modules/ParticleModule.h"
#include "Source/Runtime/Engine/Particle/Modules/ParticleModuleRequired.h"
#include "Source/Runtime/Engine/Particle/Modules/ParticleModuleSpawn.h"
#include "Source/Runtime/Engine/Particle/Modules/ParticleModuleLifetime.h"
#include "Source/Runtime/Engine/Particle/Modules/ParticleModuleSize.h"
#include "Source/Runtime/Engine/Particle/Modules/ParticleModuleVelocity.h"
#include "Source/Runtime/Engine/Particle/Modules/ParticleModuleColor.h"
#include "Source/Runtime/Core/Object/ObjectFactory.h"
#include "World.h"
#include "CameraActor.h"
#include "imgui.h"

SParticleViewerWindow::SParticleViewerWindow()
{
}

SParticleViewerWindow::~SParticleViewerWindow()
{
    if (Viewport)
    {
        delete Viewport;
        Viewport = nullptr;
    }

    if (ViewportClient)
    {
        delete ViewportClient;
        ViewportClient = nullptr;
    }

    if (PreviewWorld)
    {
        ObjectFactory::DeleteObject(PreviewWorld);
        PreviewWorld = nullptr;
    }
}

bool SParticleViewerWindow::Initialize(float StartX, float StartY, float Width, float Height, UWorld* InWorld, ID3D11Device* InDevice)
{
    Device = InDevice;
    if (!Device) return false;

    SetRect(StartX, StartY, StartX + Width, StartY + Height);

    // 1. Preview World 생성
    PreviewWorld = NewObject<UWorld>();
    PreviewWorld->SetWorldType(EWorldType::PreviewMinimal);
    PreviewWorld->Initialize();
    PreviewWorld->GetRenderSettings().DisableShowFlag(EEngineShowFlags::SF_EditorIcon);

    PreviewWorld->GetGizmoActor()->SetSpace(EGizmoSpace::Local);

    // InWorld의 설정 복사
    if (InWorld)
    {
        PreviewWorld->GetRenderSettings().SetShowFlags(InWorld->GetRenderSettings().GetShowFlags());
        PreviewWorld->GetRenderSettings().DisableShowFlag(EEngineShowFlags::SF_EditorIcon);
    }

    // 2. Viewport 생성
    Viewport = new FViewport();
    Viewport->Initialize(0, 0, 1, 1, InDevice);

    // 3. ViewportClient 생성
    ViewportClient = new FViewportClient();
    ViewportClient->SetWorld(PreviewWorld);
    ViewportClient->SetViewportType(EViewportType::Perspective);
    ViewportClient->SetViewMode(EViewMode::VMI_Lit_Phong);

    // 카메라 설정
    ACameraActor* Camera = ViewportClient->GetCamera();
    Camera->SetActorLocation(FVector(3, 0, 2));
    Camera->SetActorRotation(FVector(0, 30, -180));

    Viewport->SetViewportClient(ViewportClient);
    PreviewWorld->SetEditorCameraActor(Camera);

    return true;
}

void SParticleViewerWindow::OnRender()
{
    if (!bIsOpen)
        return;

    // ImGui 창 설정
    ImGui::SetNextWindowSize(ImVec2(1400, 900), ImGuiCond_FirstUseEver);
    if (!bInitialPlacementDone)
    {
        ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
        bInitialPlacementDone = true;
    }

    ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar;
    if (!ImGui::Begin("Particle Editor", &bIsOpen, flags))
    {
        ImGui::End();
        return;
    }

    // 1. 메뉴바
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save")) {}
            if (ImGui::MenuItem("Close")) { Close(); }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo")) {}
            if (ImGui::MenuItem("Redo")) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Asset"))
        {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Window"))
        {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help"))
        {
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    // 2. 툴바
    const float toolbarHeight = 40.0f;
    ImGui::BeginChild("Toolbar", ImVec2(0, toolbarHeight), true);
    {
        if (ImGui::Button("Save"))
        {
            SaveParticleSystem();
        }
        ImGui::SameLine();
        if (ImGui::Button("Restart Sim")) {}
        ImGui::SameLine();
        if (ImGui::Button("Undo")) {}
        ImGui::SameLine();
        if (ImGui::Button("Redo")) {}
        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();
        if (ImGui::Button("Thumbnail")) {}
        ImGui::SameLine();
        if (ImGui::Button("Bounds")) {}
    }
    ImGui::EndChild();

    // 3. 메인 컨텐츠 영역
    ImVec2 contentSize = ImGui::GetContentRegionAvail();
    const float curveEditorHeight = contentSize.y * 0.3f;
    const float mainContentHeight = contentSize.y - curveEditorHeight;
    const float leftPanelWidth = contentSize.x * 0.4f;
    const float rightPanelWidth = contentSize.x - leftPanelWidth;

    // 좌측: 뷰포트 + Properties
    ImGui::BeginChild("LeftMain", ImVec2(leftPanelWidth, mainContentHeight), false);
    {
        const float viewportHeight = mainContentHeight * 0.6f;
        const float propertiesHeight = mainContentHeight - viewportHeight;

        // 뷰포트 (헤더 + 렌더링 영역)
        ImGui::BeginChild("ViewportContainer", ImVec2(0, viewportHeight), false);
        {
            // 헤더
            const float headerHeight = 30.0f;
            ImGui::BeginChild("ViewportHeader", ImVec2(0, headerHeight), true, ImGuiWindowFlags_NoScrollbar);
            {
                ImGui::SetCursorPosY(5.0f);
                ImGui::Text("Preview");
            }
            ImGui::EndChild();

            // 뷰포트 렌더링 영역
            ImGui::BeginChild("Viewport", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar);
            {
                
             // 뷰포트에서 마우스 입력을 허용하도록 설정
             if (ImGui::IsWindowHovered())
             {
                 ImGui::GetIO().WantCaptureMouse = false;
             }
                // 뷰포트 영역 계산 (전체 Child 윈도우 영역)
                ImVec2 childPos = ImGui::GetWindowPos();
                ImVec2 childSize = ImGui::GetWindowSize();
                ImVec2 rectMin = childPos;
                ImVec2 rectMax(childPos.x + childSize.x, childPos.y + childSize.y);
                CenterRect.Left = rectMin.x;
                CenterRect.Top = rectMin.y;
                CenterRect.Right = rectMax.x;
                CenterRect.Bottom = rectMax.y;
                CenterRect.UpdateMinMax();
            }
            ImGui::EndChild();
        }
        ImGui::EndChild();

        // Properties
        ImGui::BeginChild("Particle System", ImVec2(0, propertiesHeight), true);
        {
            ImGui::Text("디테일");
            ImGui::Separator();

            if (SelectedModule)
            {
                ImGui::Text("Selected Module: %s", SelectedModule->GetClass()->Name);
                ImGui::Separator();

                // 모듈 타입별로 속성 표시
                if (auto* RequiredModule = dynamic_cast<UParticleModuleRequired*>(SelectedModule))
                {
                    ImGui::Text("Emitter Settings");
                    ImGui::DragInt("Max Particles", &RequiredModule->MaxParticles, 1.0f, 1, 10000);
                    ImGui::DragFloat("Emitter Duration", &RequiredModule->EmitterDuration, 0.01f, 0.0f, 100.0f);
                    ImGui::DragFloat("Emitter Delay", &RequiredModule->EmitterDelay, 0.01f, 0.0f, 10.0f);
                    ImGui::DragInt("Emitter Loops", &RequiredModule->EmitterLoops, 1.0f, 0, 100);
                    ImGui::DragFloat("Spawn Rate Base", &RequiredModule->SpawnRateBase, 0.1f, 0.0f, 1000.0f);
                    ImGui::Checkbox("Use Local Space", &RequiredModule->bUseLocalSpace);
                    ImGui::Checkbox("Kill On Deactivate", &RequiredModule->bKillOnDeactivate);
                    ImGui::Checkbox("Kill On Completed", &RequiredModule->bKillOnCompleted);
                }
                else if (auto* SpawnModule = dynamic_cast<UParticleModuleSpawn*>(SelectedModule))
                {
                    ImGui::Text("Spawn Settings");
                    ImGui::DragFloat("Spawn Rate Min", &SpawnModule->SpawnRate.MinValue, 0.1f, 0.0f, 1000.0f);
                    ImGui::DragFloat("Spawn Rate Max", &SpawnModule->SpawnRate.MaxValue, 0.1f, 0.0f, 1000.0f);
                    ImGui::Checkbox("Use Range", &SpawnModule->SpawnRate.bUseRange);
                }
                else if (auto* LifetimeModule = dynamic_cast<UParticleModuleLifetime*>(SelectedModule))
                {
                    ImGui::Text("Lifetime Settings");
                    ImGui::DragFloat("Lifetime Min", &LifetimeModule->Lifetime.MinValue, 0.01f, 0.0f, 100.0f);
                    ImGui::DragFloat("Lifetime Max", &LifetimeModule->Lifetime.MaxValue, 0.01f, 0.0f, 100.0f);
                    ImGui::Checkbox("Use Range", &LifetimeModule->Lifetime.bUseRange);
                }
                else if (auto* SizeModule = dynamic_cast<UParticleModuleSize*>(SelectedModule))
                {
                    ImGui::Text("Size Settings");
                    ImGui::DragFloat3("Start Size Min", &SizeModule->StartSize.MinValue.X, 1.0f, 0.0f, 1000.0f);
                    ImGui::DragFloat3("Start Size Max", &SizeModule->StartSize.MaxValue.X, 1.0f, 0.0f, 1000.0f);
                    ImGui::Checkbox("Use Range", &SizeModule->StartSize.bUseRange);
                }
                else if (auto* VelocityModule = dynamic_cast<UParticleModuleVelocity*>(SelectedModule))
                {
                    ImGui::Text("Velocity Settings");
                    ImGui::DragFloat3("Start Velocity Min", &VelocityModule->StartVelocity.MinValue.X, 1.0f, -1000.0f, 1000.0f);
                    ImGui::DragFloat3("Start Velocity Max", &VelocityModule->StartVelocity.MaxValue.X, 1.0f, -1000.0f, 1000.0f);
                    ImGui::Checkbox("Use Range", &VelocityModule->StartVelocity.bUseRange);
                    ImGui::DragFloat3("Gravity", &VelocityModule->Gravity.X, 1.0f, -10000.0f, 10000.0f);
                }
                else if (auto* ColorModule = dynamic_cast<UParticleModuleColor*>(SelectedModule))
                {
                    ImGui::Text("Color Settings");
                    ImGui::ColorEdit3("Start Color Min", &ColorModule->StartColor.MinValue.R);
                    ImGui::ColorEdit3("Start Color Max", &ColorModule->StartColor.MaxValue.R);
                    ImGui::Checkbox("Use Range", &ColorModule->StartColor.bUseRange);
                    ImGui::DragFloat("Start Alpha Min", &ColorModule->StartAlpha.MinValue, 0.01f, 0.0f, 1.0f);
                    ImGui::DragFloat("Start Alpha Max", &ColorModule->StartAlpha.MaxValue, 0.01f, 0.0f, 1.0f);
                }
            }
            else if (CurrentParticleSystem)
            {
                ImGui::TextDisabled("No module selected");
            }
            else
            {
                ImGui::TextDisabled("No particle system loaded");
            }
        }
        ImGui::EndChild();
    }
    ImGui::EndChild();

    ImGui::SameLine();

    // 우측: Emitter/모듈 패널
    ImGui::BeginChild("EmitterPanel", ImVec2(rightPanelWidth, mainContentHeight), true);
    {
        ImGui::Text("이미터");
        ImGui::Separator();

        if (CurrentParticleSystem && CurrentParticleSystem->Emitters.Num() > 0)
        {
            // 각 이미터를 블럭 형태로 표시
            for (int i = 0; i < CurrentParticleSystem->Emitters.Num(); i++)
            {
                UParticleEmitter* Emitter = CurrentParticleSystem->Emitters[i];
                if (!Emitter) continue;

                ImGui::PushID(i);

                // 이미터 블럭 (가로만 고정, 세로는 전체)
                const float emitterBlockWidth = 200.0f;
                ImGui::BeginChild("EmitterBlock", ImVec2(emitterBlockWidth, 0), true);
                {
                    // 이미터 헤더 (Selectable로 호버링 가능하게)
                    const float headerHeight = 50.0f;
                    ImGui::PushID("emitter_header");
                    if (ImGui::Selectable("##emitterheader", false, 0, ImVec2(0, headerHeight)))
                    {
                        // 이미터 헤더 클릭 시 처리
                    }
                    ImGui::PopID();

                    // 헤더 위에 내용 그리기
                    ImVec2 headerMin = ImGui::GetItemRectMin();
                    ImVec2 headerMax = ImGui::GetItemRectMax();

                    // 이미터 이름
                    ImGui::SetCursorScreenPos(ImVec2(headerMin.x + 5, headerMin.y + 5));
                    ImGui::Text("%s", Emitter->GetName().c_str());

                    // 미리보기 박스 (우측 하단)
                    ImGui::SetCursorScreenPos(ImVec2(headerMax.x - 45, headerMin.y + 5));
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
                    ImGui::Button("##preview", ImVec2(40, 40));
                    ImGui::PopStyleColor(3);

                    // 원래 커서 위치 복구
                    ImGui::SetCursorScreenPos(ImVec2(headerMin.x, headerMax.y));

                    ImGui::Separator();

                    // 모듈 리스트
                    if (Emitter->LODLevels.Num() > 0)
                    {
                        UParticleLODLevel* LOD = Emitter->LODLevels[0];
                        if (LOD)
                        {
                            // 필수 모듈
                            if (LOD->RequiredModule)
                            {
                                ImGui::PushID("req");
                                bool isSelected = (SelectedModule == LOD->RequiredModule);
                                if (ImGui::Selectable(LOD->RequiredModule->GetClass()->Name, isSelected, 0, ImVec2(0, 20)))
                                {
                                    SelectedModule = LOD->RequiredModule;
                                }
                                ImGui::PopID();
                            }

                            // 스폰 모듈
                            if (LOD->SpawnModule)
                            {
                                ImGui::PushID("spawn");
                                bool isSelected = (SelectedModule == LOD->SpawnModule);
                                if (ImGui::Selectable(LOD->SpawnModule->GetClass()->Name, isSelected, 0, ImVec2(0, 20)))
                                {
                                    SelectedModule = LOD->SpawnModule;
                                }
                                ImGui::PopID();
                            }

                            // SpawnModules
                            for (int m = 0; m < LOD->SpawnModules.Num(); m++)
                            {
                                UParticleModule* Module = LOD->SpawnModules[m];
                                if (Module)
                                {
                                    ImGui::PushID(m);
                                    bool isSelected = (SelectedModule == Module);
                                    if (ImGui::Selectable(Module->GetClass()->Name, isSelected, 0, ImVec2(0, 20)))
                                    {
                                        SelectedModule = Module;
                                    }
                                    ImGui::PopID();
                                }
                            }

                            // UpdateModules
                            for (int m = 0; m < LOD->UpdateModules.Num(); m++)
                            {
                                UParticleModule* Module = LOD->UpdateModules[m];
                                if (Module)
                                {
                                    ImGui::PushID(m + 1000);
                                    bool isSelected = (SelectedModule == Module);
                                    if (ImGui::Selectable(Module->GetClass()->Name, isSelected, 0, ImVec2(0, 20)))
                                    {
                                        SelectedModule = Module;
                                    }
                                    ImGui::PopID();
                                }
                            }
                        }
                    }
                }
                ImGui::EndChild();

                ImGui::PopID();

                // 다음 이미터를 옆에 배치 (2열 레이아웃)
                if ((i + 1) % 2 != 0 && i + 1 < CurrentParticleSystem->Emitters.Num())
                {
                    ImGui::SameLine();
                }
            }
        }
        else
        {
            ImGui::TextDisabled("No emitters");
        }
    }
    ImGui::EndChild();

    // 4. 하단: Curve Editor
    ImGui::BeginChild("CurveEditor", ImVec2(0, curveEditorHeight), true);
    {
        ImGui::Text("Curve Editor");
        ImGui::Separator();

        // 커브 편집 툴바
        if (ImGui::Button("Horizontal")) {}
        ImGui::SameLine();
        if (ImGui::Button("Vertical")) {}
        ImGui::SameLine();
        if (ImGui::Button("All")) {}
        ImGui::SameLine();
        if (ImGui::Button("Selected")) {}
        ImGui::SameLine();
        if (ImGui::Button("Pan")) {}
        ImGui::SameLine();
        if (ImGui::Button("Zoom")) {}

        ImGui::Separator();
        ImGui::TextDisabled("Curve editing area (Coming Soon)");
    }
    ImGui::EndChild();

    ImGui::End();
}

void SParticleViewerWindow::OnUpdate(float DeltaSeconds)
{
    if (!Viewport || !ViewportClient)
        return;

    // ViewportClient 업데이트
    if (ViewportClient)
    {
        ViewportClient->Tick(DeltaSeconds);
    }

    // Preview World 업데이트
    if (PreviewWorld)
    {
        PreviewWorld->Tick(DeltaSeconds);
    }
}

void SParticleViewerWindow::OnMouseMove(FVector2D MousePos)
{
    if (!Viewport) return;

    if (CenterRect.Contains(MousePos))
    {
        FVector2D LocalPos = MousePos - FVector2D(CenterRect.Left, CenterRect.Top);
        Viewport->ProcessMouseMove((int32)LocalPos.X, (int32)LocalPos.Y);
    }
}

void SParticleViewerWindow::OnMouseDown(FVector2D MousePos, uint32 Button)
{
    if (!Viewport) return;

    if (CenterRect.Contains(MousePos))
    {
        FVector2D LocalPos = MousePos - FVector2D(CenterRect.Left, CenterRect.Top);
        Viewport->ProcessMouseButtonDown((int32)LocalPos.X, (int32)LocalPos.Y, (int32)Button);
    }
}

void SParticleViewerWindow::OnMouseUp(FVector2D MousePos, uint32 Button)
{
    if (!Viewport) return;

    if (CenterRect.Contains(MousePos))
    {
        FVector2D LocalPos = MousePos - FVector2D(CenterRect.Left, CenterRect.Top);
        Viewport->ProcessMouseButtonUp((int32)LocalPos.X, (int32)LocalPos.Y, (int32)Button);
    }
}

void SParticleViewerWindow::OnRenderViewport()
{
    if (Viewport && CenterRect.GetWidth() > 0 && CenterRect.GetHeight() > 0)
    {
        const uint32 NewStartX = static_cast<uint32>(CenterRect.Left);
        const uint32 NewStartY = static_cast<uint32>(CenterRect.Top);
        const uint32 NewWidth  = static_cast<uint32>(CenterRect.Right - CenterRect.Left);
        const uint32 NewHeight = static_cast<uint32>(CenterRect.Bottom - CenterRect.Top);

        Viewport->Resize(NewStartX, NewStartY, NewWidth, NewHeight);

        // 뷰포트 렌더링 (ImGui보다 먼저)
        Viewport->Render();
    }
}

void SParticleViewerWindow::LoadParticleSystem(const FString& Path)
{
    // TODO: 경로에서 파티클 시스템 로드
}

void SParticleViewerWindow::LoadParticleSystem(UParticleSystem* ParticleSystem)
{
    CurrentParticleSystem = ParticleSystem;
}

void SParticleViewerWindow::SaveParticleSystem()
{
    if (!CurrentParticleSystem)
    {
        UE_LOG("No particle system to save");
        return;
    }

    // SavePath가 설정되어 있으면 (새로 생성한 경우) SavePath에 저장
    if (!SavePath.empty())
    {
        if (CurrentParticleSystem->SaveToFile(SavePath))
        {
            UE_LOG("Particle system saved to: %s", SavePath.c_str());
        }
        else
        {
            UE_LOG("Failed to save particle system");
        }
    }
    else
    {
        // TODO: 파일 다이얼로그를 열어서 저장 경로 선택
        UE_LOG("SavePath is not set. Please implement file dialog.");
    }
}