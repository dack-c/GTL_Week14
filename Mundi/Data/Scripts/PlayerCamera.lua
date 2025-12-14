-------------------------------------------------

local Camera
local OriginFOV

local CurSpeed
local LastPos

local FOVLerpValue = 0.01
local MaxAdditionalFOV = 14
local MinFOVSpeed = 0.05
local MaxFOVSpeed = 0.2 --최대속도가 아니라 fov가 최대가 되는 속도
function BeginPlay()  
  Camera = GetComponent(Obj, "UCameraComponent")
  OriginFOV = Camera:GetFOV()
  LastPos = Obj.Location
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
--속도구하기
local CurPos = Obj.Location
local MoveVt = CurPos - LastPos
CurSpeed = MoveVt:Size()
LastPos = CurPos

local SpeedFactor = CurSpeed / (MaxFOVSpeed - MinFOVSpeed)
SpeedFactor = Util.Clamp(SpeedFactor,0,1)
local AddFOV = SpeedFactor * MaxAdditionalFOV
local FOVDesti = OriginFOV + AddFOV
local CurFOV = Camera:GetFOV()
Camera:SetFOV(Util.Lerp(CurFOV, FOVDesti, FOVLerpValue))
end



