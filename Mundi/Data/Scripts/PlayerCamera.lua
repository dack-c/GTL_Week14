-------------------------------------------------

local PlayerActor
local CapsuleComponent
local CharacterMoveComp = nil
local SkeletalMesh = nil
local AnimInstance = nil
local AnimStateMachine = nil

function BeginPlay()  
  print("Begin")

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
