-- CinematicTrigger.lua
-- 트리거 진입 시 시네마틱 카메라로 전환하고 슬로우 모션을 활성화

-- 전역 시네마틱 상태 관리
if not _G.CinematicState then
    _G.CinematicState = {
        bIsActive = false,
        OriginalCamera = nil,
        CinematicDuration = 0.0,
        ElapsedTime = 0.0,
        OriginalMotionBlurIntensity = 0.0
    }
end

local CinematicCameraName = "TargetCameraActor"  -- 시네마틱 카메라 이름
local CinematicCameraComponentName = "TargetCameraComponent" -- FIXME: GetComponent가 동작하지 않아 이름으로 직접 찾음
local SlowMotionScale = 0.3              -- 슬로우 모션 배율 (0.3 = 30% 속도)
local Duration = 1.0                      -- 시네마틱 지속 시간 (초)
local CameraOffset = Vector(-50, -150, 80)   -- 카메라 오프셋 (플레이어 기준: 뒤쪽, 왼쪽, 위쪽)

function BeginPlay()
    print("CinematicTrigger: Camera set to " .. CinematicCameraName)
end

function Tick(dt)
    -- 시네마틱이 활성화되어 있으면 타이머 업데이트
    if _G.CinematicState.bIsActive then
        _G.CinematicState.ElapsedTime = _G.CinematicState.ElapsedTime + dt

        -- 지속 시간이 지나면 시네마틱 종료
        if _G.CinematicState.ElapsedTime >= _G.CinematicState.CinematicDuration then
            EndCinematic()
        end
    end
end

function OnBeginOverlap(OtherActor)
    -- 이미 시네마틱이 활성화되어 있으면 무시
    if _G.CinematicState.bIsActive then
        return
    end

    StartCinematic()
end

function OnEndOverlap(OtherActor)
    -- 트리거를 벗어나면 시네마틱 종료
    if _G.CinematicState.bIsActive then
        EndCinematic()
    end
end

function StartCinematic()
    print("Cinematic: Start")

    -- 원래 카메라 저장
    _G.CinematicState.OriginalCamera = GetCamera()

    -- FIXME: GetComponent가 동작하지 않아 이름으로 직접 찾음
    local CinematicCamera = FindComponentByName(CinematicCameraComponentName)
    if CinematicCamera == nil then
        print("[Cinematic] Error: Camera component not found by name - " .. CinematicCameraComponentName)
        -- 시네마틱 실패 시 플래그 원상 복구
        GlobalConfig.bIsCinematicActive = false
        return
    end

    -- 카메라 전환
    SetViewTargetCamera(CinematicCamera)

    -- 슬로우 모션 활성화
    SetTimeDilation(SlowMotionScale)

    -- 상태 업데이트
    _G.CinematicState.bIsActive = true
    _G.CinematicState.CinematicDuration = Duration
    _G.CinematicState.ElapsedTime = 0.0

    print("Cinematic: Camera switched, SlowMotion " .. SlowMotionScale .. "x, Motion Blur OFF")
end

function EndCinematic()
    print("Cinematic: End")

    -- 원래 카메라로 복귀
    if _G.CinematicState.OriginalCamera ~= nil then
        SetViewTargetCamera(_G.CinematicState.OriginalCamera)
    end

    -- 정상 속도 복원
    SetTimeDilation(1.0)

    -- 상태 리셋
    _G.CinematicState.bIsActive = false
    _G.CinematicState.ElapsedTime = 0.0
    _G.CinematicState.CinematicCameraActor = nil

    print("Cinematic: Camera restored, Normal speed")
end

-- 에디터에서 카메라 이름 설정용 헬퍼 함수
function SetCinematicCameraName(Name)
    CinematicCameraName = Name
    print("Cinematic: Camera name set to " .. CinematicCameraName)
end

function SetCinematicDuration(NewDuration)
    Duration = NewDuration
    print("Cinematic: Duration set to " .. Duration .. " seconds")
end

function SetSlowMotionScale(Scale)
    SlowMotionScale = Scale
    print("Cinematic: SlowMotion scale set to " .. Scale)
end
