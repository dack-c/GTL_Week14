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
 local Pos = Vector2D(500.0,500.0)
local Size = Vector2D(300.0,400.0)
local RectTransform = RectTransform(Pos,Size)
local Color = Vector4(1,1,1,1)
RectTransform.Pivot = Vector2D(0.5,0.5)
--DrawUIText(RectTransform, "asdf", Color, 50)
RectTransform.ZOrder = 0;
DrawUISprite(RectTransform, "Data/Textures/GreenLight.png", 0.5)

RectTransform.ZOrder = 1;

DrawUISprite(RectTransform, "Data/Textures/Boom.png", 1.0)
RectTransform.ZOrder = 2;
RectTransform.Anchor = Vector2D(0.5,0.5)
RectTransform.Pivot = Vector2D(1,1)
RectTransform.Pos = Vector2D(0,0)
DrawUISprite(RectTransform, "Data/Textures/GreenLight.png", 0.5)
RectTransform.Pivot = Vector2D(0.5,0.5)
DrawUISprite(RectTransform, "Data/Textures/GreenLight.png", 0.5)



end

function CameraAction()
CameraManager:SetViewTargetWithBlend(StartSequenceCam_1, 2)
end