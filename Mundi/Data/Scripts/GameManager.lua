-- 전역 GlobalConfig 테이블을 전역에 초기화
if not _G.GlobalConfig then
    _G.GlobalConfig = {
        GameState = "Init",
        bIsGameClear = false,
        bIsPlayerDeath = false
    }
end
GlobalConfig = _G.GlobalConfig  -- 로컬 별칭

function BeginPlay()
    InitGame()
    GlobalConfig.GameState = "Init"
    PlaySound2DByFile("Data/Audio/pakourBGM.wav")
end

function EndPlay()
    StopAllSounds()
end

function OnBeginOverlap(OtherActor)
end

function OnEndOverlap(OtherActor)
end

local PlayTime = 0

-- 게임 상태 초기화
function InitGame()
    -- TODO: 플레이어 생성
    PlayTime = 0

    local Player = GetPlayer()
    if Player == nil then
        return
    end

    GetComponent(Player, "USkeletalMeshComponent"):SetRagdoll(false)
    GetComponent(Player, "USpringArmComponent").CameraLagSpeed = 0.05
    Player.Location = GetStartPosition()
    GetComponent(Player, "UCharacterMovementComponent"):ResetVelocity()

    -- Capsule Offset 초기화 (Player.lua의 로컬 함수 대신 직접 처리)
    local CharacterMoveComp = GetComponent(Player, "UCharacterMovementComponent")
    if CharacterMoveComp then
        CharacterMoveComp.CapsuleOffset = Vector(0,0,0)
        CharacterMoveComp:SetUseGravity(true)
    end
end


CurVisibilty = true
function Tick(dt)

    -- 강제 리플레이
    if InputManager:IsKeyDown("P") then
        InitGame()
        GlobalConfig.GameState = "Init"
    end

    -- 히든 디버그: G 키로 골 근처 스폰
    if InputManager:IsKeyDown("G") then
        local GoalPos = Vector(20, 196, -25)  -- 골 근처 테스트 위치
        GetPlayer().Location = GoalPos
        GetComponent(GetPlayer(), "UCharacterMovementComponent"):ResetVelocity()
    end

    if GlobalConfig.GameState == "Init" then
        RenderInitUI()

        if InputManager:IsKeyDown("Q") then
            GlobalConfig.GameState = "Start"
            InitGame()
        end
    
    elseif GlobalConfig.GameState == "Start" then
        -- 시작 연출 이 끝나면 Playing 으로 전환됨
        GlobalConfig.bIsGameClear = false
        GlobalConfig.bIsPlayerDeath = false
        GlobalConfig.GameState = "Playing"

        -- 플레이어 입력 활성화
        SetPlayerInputEnabled(true)

    elseif GlobalConfig.GameState == "Playing" then
        PlayTime = PlayTime + dt
        RenderInGameUI()

        -- 클리어 체크가 먼저 (우선순위 높음)
        if GlobalConfig.bIsGameClear == true then
            GlobalConfig.GameState = "Clear"
            GetComponent(GetPlayer(), "USpringArmComponent").CameraLagSpeed = 0

            -- 플레이어 입력 차단
            SetPlayerInputEnabled(false)
        -- 사망 체크
        elseif GlobalConfig.bIsPlayerDeath == true then
            GlobalConfig.GameState = "Death"
            GetComponent(GetPlayer(), "USkeletalMeshComponent"):SetRagdoll(true)
            GetComponent(GetPlayer(), "USpringArmComponent").CameraLagSpeed = 0
            PlaySound2DOneShotByFile("Data/Audio/Scream.wav")

            -- 플레이어 입력 차단
            SetPlayerInputEnabled(false)
        -- 아직 클리어/사망 안 했을 때만 낙사 판정
        else
            -- 골 높이보다 낮게 떨어지면 낙사 처리
            local PlayerPos = GetPlayer().Location
            local GoalHeight = -34.777500  -- 골 지점의 Z 좌표
            local GoalPosX = -31.036255
            local GoalPosY = 190.940994
            local GoalRadius = 35.0  -- 골 트리거 범위 (BoxExtent 30 + 여유 5)

            -- 골 트리거 영역 밖에 있는지 체크
            local DistanceToGoal = math.sqrt((PlayerPos.X - GoalPosX)^2 + (PlayerPos.Y - GoalPosY)^2)
            local bIsOutsideGoalArea = DistanceToGoal > GoalRadius

            -- 골 영역 밖에서 골 높이보다 낮으면 낙사
            if bIsOutsideGoalArea and PlayerPos.Z < GoalHeight then
                GlobalConfig.bIsPlayerDeath = true
            end
        end

    elseif GlobalConfig.GameState == "Death" then
        RenderDeathUI()

        if InputManager:IsKeyDown("E") then
            GlobalConfig.GameState = "Init"
        end

    elseif GlobalConfig.GameState == "Clear" then
        -- 클리어 타임 표시
        RenderClearUI()

        if InputManager:IsKeyDown("E") then
            GlobalConfig.GameState = "Init"
        end

    end
end

function WaitAndInit()
    coroutine.yield("wait_time", 1.0)
    GlobalConfig.GameState = "Init"
end

function SetCursorVisible(Show)
    InputManager:SetCursorVisible(Show)
    CurVisibilty = Show
end

-- 시작 UI 출력
function RenderInitUI()
    local Rect = RectTransform()
    local Color = Vector4(1,1,1,1)

    local NeonCyan = Vector4(0.0, 1.0, 1.0, 1.0)  -- 네온 시안

    local AnchorMin = Vector2D(0,0)
    local AnchorMax = Vector2D(1,1)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin,AnchorMax)
    DrawUISprite(Rect, "Data/UI/Main.png", 1.0)

    -- Rect.Pos = Vector2D(0.0,0.0)
    -- Rect.Size = Vector2D(1264.0,848.0)
    -- Rect.Pivot = Vector2D(0.0,0.0)
    -- Rect.ZOrder = 0;
    -- DrawUISprite(Rect, "Data/UI/Main.png", 1.0)

    AnchorMin = Vector2D(0,0.6)
    AnchorMax = Vector2D(1,0.8)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin,AnchorMax)
    Rect.ZOrder = 1;
    DrawUIText(Rect, "PRESS Q TO START", NeonCyan, 40, "Platinum Sign")

    AnchorMin = Vector2D(0,0.8)
    AnchorMax = Vector2D(1,1)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin,AnchorMax)
    Rect.ZOrder = 1;
    DrawUIText(Rect, "김상천, 김진철, 김호민, 김희준", Color, 60, "THEFACESHOP INKLIPQUID")
end

-- 인게임 UI 출력
function RenderInGameUI()
    local Rect = RectTransform()
    local Color = Vector4(0,1,0,1)

    AnchorMin = Vector2D(0,0)
    AnchorMax = Vector2D(0.3,0.2)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin,AnchorMax)
    Rect.ZOrder = 1;

    local RemainHeight = (-39 - GetPlayer().Location.Z) * -1;
    if RemainHeight < 0 then
        RemainHeight = 0
    end

    DrawUIText(Rect, "남은 높이: "..string.format("%.1f", RemainHeight).."m", Color, 30, "THEFACESHOP INKLIPQUID")


    AnchorMin = Vector2D(0.7,0)
    AnchorMax = Vector2D(1,0.2)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin,AnchorMax)
    Color = Vector4(1,0.5,0.5,1)
    DrawUIText(Rect, "플레이 시간: "..string.format("%.1f", PlayTime).."초", Color, 30, "THEFACESHOP INKLIPQUID")
end

-- 사망 UI 출력
function RenderDeathUI()
    local Rect = RectTransform()
    local Color = Vector4(1,0,0,1)

    -- 낙사 텍스트
    local AnchorMin = Vector2D(0,0.4)
    local AnchorMax = Vector2D(1,0.7)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin,AnchorMax)
    Rect.ZOrder = 1;
    DrawUIText(Rect, "낙사", Color, 100, "THEFACESHOP INKLIPQUID")

    -- 재시작 안내 텍스트
    AnchorMin = Vector2D(0,0.2)
    AnchorMax = Vector2D(1,0.4)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin,AnchorMax)
    Rect.ZOrder = 1;
    local NeonPink = Vector4(1.0, 0.1, 0.7, 1.0)  -- 네온 핑크
    DrawUIText(Rect, "PRESS E TO RESTART", NeonPink, 40, "Platinum Sign")
end

-- 클리어 UI 출력
function RenderClearUI()
    local Rect = RectTransform()
    local Color = Vector4(0,1,1,1)

    -- local AnchorMin = Vector2D(0,0)
    -- local AnchorMax = Vector2D(1,1)
    -- Rect = FRectTransform.CreateAnchorRange(AnchorMin,AnchorMax)
    -- DrawUISprite(Rect, "Data/UI/Main.png", 1.0)

    -- -- Rect.Pos = Vector2D(0.0,0.0)
    -- -- Rect.Size = Vector2D(1264.0,848.0)
    -- -- Rect.Pivot = Vector2D(0.0,0.0)
    -- -- Rect.ZOrder = 0;
    -- -- DrawUISprite(Rect, "Data/UI/Main.png", 1.0)
    
    AnchorMin = Vector2D(0,0)
    AnchorMax = Vector2D(1,1)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin,AnchorMax)
    Rect.ZOrder = 1;
    DrawUIText(Rect, "클리어!\n"..string.format("%.1f", PlayTime).."초", Color, 80, "THEFACESHOP INKLIPQUID")
end
