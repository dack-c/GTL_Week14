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
  if Cam1Actor == nil then
  print("없")
  else
  print("있")
  end

  if StartSequenceCam_1 == nil then
  print("없2")
  else
  print("있2")
  end
end

function EndPlay()

end

function OnBeginOverlap(OtherActor)
print("beginoverlap")
end

function OnEndOverlap(OtherActor)
     print("OnEndOverlap")

end

function Tick(Delta)

end

function CameraAction()
print("asdfadad")

CameraManager:SetViewTargetWithBlend(StartSequenceCam_1, 2)
end