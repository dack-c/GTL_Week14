-------------------------------------------------

local PlayerActor
local CapsuleComponent
function BeginPlay()  
  print("Begin")
  PlayerActor = FindObjectByName("캐릭터_0")
  CapsuleComponent = GetComponent(PlayerActor, "UCapsuleComponent")

  if PlayerActor == nil then
    print("PlayerActor is nil")
  end

  if CapsuleComponent == nil then
    print("CapsuleComponent is nil")
  else
    print(CapsuleComponent.CapsuleHalfHeight)
    print(CapsuleComponent.CapsuleRadius)
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
