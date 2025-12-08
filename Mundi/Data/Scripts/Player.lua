-------------------------------------------------

local PlayerActor
local CapsuleComponent
local CharacterMoveComp = nil
local SkeletalMesh = nil
local AnimInstance = nil
local AnimStateMachine = nil

function BeginPlay()  
  print("Begin")
  PlayerActor = FindObjectByName("캐릭터_0")
  CapsuleComponent = GetComponent(PlayerActor, "UCapsuleComponent")
  CharacterMoveComp = GetComponent(PlayerActor, "UCharacterMovementComponent")
  SkeletalMesh = GetComponent(PlayerActor, "USkeletalMeshComponent")

  if PlayerActor == nil then
    print("PlayerActor is nil")
  end

  if CapsuleComponent == nil then
    print("CapsuleComponent is nil")
  else
    print(CapsuleComponent.CapsuleHalfHeight)
    print(CapsuleComponent.CapsuleRadius)
  end

  -- CharacterMoveComp.CapsuleOffset = Vector(0,0,0.8)

  if SkeletalMesh then
        print(" -> SkeletalMesh found.")

        -- C++: GetAnimInstance() -> Lua: :GetAnimInstance()
        -- if SkeletalMesh.GetAnimInstance then
        --     AnimInstance = SkeletalMesh:GetAnimInstance()
        -- else
        --     print("[Error] SkeletalMesh does not have GetAnimInstance function.")
        -- end
        AnimInstance = SkeletalMesh:GetAnimInstance()
  end

  if AnimInstance ~= nil then
      print(" -> AnimInstance found and cached successfully.")
  else
      print("[Warning] AnimInstance is nil. Check if Animation Blueprint is assigned.")
  end

  AnimStateMachine = AnimInstance:GetStateMachine()

  if AnimStateMachine then
      print(" -> AnimStateMachine found.")
  else
      print("[Warning] AnimStateMachine is nil.")
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
  local CurAnimName = AnimStateMachine:GetCurrentState()
  print("Current Animation State: " .. CurAnimName)
end
