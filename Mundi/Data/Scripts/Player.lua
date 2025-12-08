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
        -- AnimInstance = SkeletalMesh:GetAnimInstance()
        AnimInstance = GetAnimInstanceOfSkeletal(SkeletalMesh)
  end

  if AnimInstance ~= nil then
      print(" -> AnimInstance found and cached successfully.")
  else
      print("[Warning] AnimInstance is nil. Check if Animation Blueprint is assigned.")
  end

  -- AnimStateStr = AnimInstance:GetCurrentStateName()
  -- print("Current Animation State Name: " .. AnimStateStr)

  -- AnimStateMachine = AnimInstance:GetStateMachine()

  -- if AnimStateMachine then
  --     print(" -> AnimStateMachine found.")
  -- else
  --     print("[Warning] AnimStateMachine is nil.")
  -- end

end

function EndPlay()

end

function OnBeginOverlap(OtherActor)
print("beginoverlap")
end

function OnEndOverlap(OtherActor)
     print("OnEndOverlap")

end

local PreAnimStateStr = ""
local bChangedFromClimb = false
local PostDelta = 0.0
function Tick(Delta)
  AnimInstance = GetAnimInstanceOfSkeletal(SkeletalMesh)
  -- local CurAnimName = AnimStateMachine:GetCurrentState()
  -- print("Current Animation State: " .. CurAnimName)
  AnimStateStr = AnimInstance:GetCurrentStateName()
  -- print("Current Animation State Name: " .. AnimStateStr)

  if bChangedFromClimb == true then
    PostDelta = PostDelta + Delta
    if PostDelta > 0.35 then
      bChangedFromClimb = false
      PostDelta = 0.0
    else
      CharacterMoveComp.CapsuleOffset = Vector(0.5,0,0.0)
    end
  end

  if AnimStateStr == "Vault" then
    PreAnimStateStr = "Vault"
    CharacterMoveComp.CapsuleOffset = Vector(0,0,0.3)
    -- CharacterMoveComp.CapsuleOffset = Vector(0,0,5.0)
    -- print("Vaulting - Adjusting Capsule  Z Offset: " .. CharacterMoveComp.CapsuleOffset.Z)
  elseif AnimStateStr == "Climb" then
    PreAnimStateStr = "Climb"
    if AnimInstance:GetCurrentPlayTime() > 3.8 then
      CharacterMoveComp.CapsuleOffset = Vector(0.5,0,0.0)
    else
      CharacterMoveComp.CapsuleOffset = Vector(0.0,0,2.0)
    end
  elseif AnimStateStr ~= "Climb" and PreAnimStateStr == "Climb" then
    bChangedFromClimb = true
    PreAnimStateStr = AnimStateStr
    CharacterMoveComp.CapsuleOffset = Vector(0.5,0,0.0)
  elseif bChangedFromClimb == false then
    CharacterMoveComp.CapsuleOffset = Vector(0,0,0)
  end


  -- print("AnimStateStr: " .. AnimStateStr .. " | Capsule Z Offset: " .. CharacterMoveComp.CapsuleOffset.Z)
end
