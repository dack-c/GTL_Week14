-- GameEvent.lua는 _G.GlobalConfig를 참조
GlobalConfig = _G.GlobalConfig

-- 각 함수가 이미 호출되었는지 기억하는 플래그를 전역으로 관리
if not _G.GameEventFlags then
  _G.GameEventFlags = {
    bClearCalled = false,
    bDeathCalled = false
  }
end

function BeginPlay()
  -- PIE 시작 시 플래그 리셋
  ResetEventFlags()
end

function ResetEventFlags()
  _G.GameEventFlags.bClearCalled = false
  _G.GameEventFlags.bDeathCalled = false
end

function Tick(dt)
end

function OnBeginOverlap(OtherActor)
    -- 골 트리거일 경우 클리어 처리
    DoClear()
end

function OnEndOverlap(OtherActor)
end

function DoClear()
  -- 이미 호출되었으면 무시
  if _G.GameEventFlags.bClearCalled then
    return
  end

  _G.GameEventFlags.bClearCalled = true
  GlobalConfig.bIsGameClear = true
  print("Game Clear")
end

function DoDeath()
  -- 이미 호출되었으면 무시
  if _G.GameEventFlags.bDeathCalled then
    return
  end

  _G.GameEventFlags.bDeathCalled = true
  GlobalConfig.bIsPlayerDeath = true
  print("Player Death")
end
