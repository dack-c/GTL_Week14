-------------------------------------------------
function BeginPlay()  
end

function EndPlay()
end

function OnBeginOverlap(OtherActor)
end

function OnEndOverlap(OtherActor)
end

local TutorialImagePath
local TutorialImageTime = 0.0
local TutorialMoveSuccess = false

function Tick(Delta)

if TutorialImageTime > 0 then
 local Pos = Vector2D(0.0,300.0)
local Size = Vector2D(300.0,400.0)
local RectTransform = RectTransform(Pos,Size)
RectTransform.Pivot = Vector2D(0.5,0.5)
RectTransform.Anchor = Vector2D(0.5,0)
local Opacity = GetOpacity(TutorialImageTime)
DrawUISprite(RectTransform, TutorialImagePath, Opacity)
TutorialImageTime = TutorialImageTime - Delta
end
end


function GetOpacity(Time)
if Time > 4 then
return 1 - (Time - 4)
elseif Time > 1 then
return 1
else
return Time / 1
end
end

function TutorialMove()
if TutorialMoveSuccess == true then
return
end
TutorialMoveSuccess = true
TutorialImagePath = "Data/Textures/MoveTutorialTemp.png"
TutorialImageTime = 5
end


local TutorialJumpSuccess = false
function TutorialJump()
if TutorialJumpSuccess == true then
return
end
TutorialJumpSuccess = true
TutorialImagePath = "Data/Textures/JumpTutorialTemp.png"
TutorialImageTime = 5
end


local TutorialSlideSuccess = false
function TutorialSlide()
if TutorialSlideSuccess == true then
return
end
TutorialSlideSuccess = true
TutorialImagePath = "Data/Textures/SlideTutorialTemp.png"
TutorialImageTime = 5
end


local TutorialEndSuccess = false
function TutorialEnd()
if TutorialEndSuccess == true then
return
end
TutorialEndSuccess = true
TutorialImagePath = "Data/Textures/QETutorialTemp.png"
TutorialImageTime = 5
end