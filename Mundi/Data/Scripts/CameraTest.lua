local CameraManager
local PlayerCam
local StartSequenceCam_1
local Cam2
local Cam3

-------------------------------------------------
function BeginPlay()  

    CameraManager = GetCameraManager()
  local PlayerActor = FindObjectByName("캐릭터_0")
  PlayerCam = GetComponent(PlayerActor, "UCameraComponent")
  local Cam1Actor = FindObjectByName("StartSequenceCam_1")
  StartSequenceCam_1 = GetComponent(Cam1Actor, "UCameraComponent")

end

function EndPlay()

end

function OnBeginOverlap(OtherActor)
end

function OnEndOverlap(OtherActor)
end

function Tick(Delta)

  local Pos = Vector2D(100.0,100.0)
    local Size = Vector2D(100.0,100.0)
    local RectTransform = RectTransform(Pos,Size)
    local Color = Vector4(1,1,1,1)
    DrawUIText(RectTransform, "asdf", Color)
end

function CameraAction()
CameraManager:SetViewTargetWithBlend(StartSequenceCam_1, 2)
end