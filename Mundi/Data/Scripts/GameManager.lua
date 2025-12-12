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

    -- GameEvent 플래그 리셋 (재시작 시 클리어/사망 이벤트 재활성화)
    if _G.GameEventFlags then
        _G.GameEventFlags.bClearCalled = false
        _G.GameEventFlags.bDeathCalled = false
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
        -- 사망
        elseif GlobalConfig.bIsPlayerDeath == true then
            GlobalConfig.GameState = "Death"
            GetComponent(GetPlayer(), "USkeletalMeshComponent"):SetRagdoll(true)
            GetComponent(GetPlayer(), "USpringArmComponent").CameraLagSpeed = 0
            PlaySound2DOneShotByFile("Data/Audio/Scream.wav")

            -- 랜덤 사망 문구 선택
            local RandomIndex = RandomInt(1, #DeathMessages)
            CurrentDeathMessage = DeathMessages[RandomIndex]

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
            CurrentDeathMessage = ""  -- 사망 메시지 리셋
        end

    elseif GlobalConfig.GameState == "Clear" then
        -- 클리어 타임 표시
        RenderClearUI()

        if InputManager:IsKeyDown("E") then
            GlobalConfig.GameState = "Init"
            CurrentDeathMessage = ""  -- 사망 메시지 리셋
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

    -- PRESS Q TO START 배경 박스
    AnchorMin = Vector2D(0, 0.65)
    AnchorMax = Vector2D(1, 0.75)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin, AnchorMax)
    Rect.ZOrder = 0
    DrawUISprite(Rect, "Data/UI/BlackBox.png", 0.3)

    -- PRESS Q TO START 텍스트
    AnchorMin = Vector2D(0,0.6)
    AnchorMax = Vector2D(1,0.8)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin,AnchorMax)
    Rect.ZOrder = 1;
    DrawUIText(Rect, "PRESS Q TO START", NeonCyan, 40, "Platinum Sign")

    RenderCredits()
end

-- 인게임 UI 출력
function RenderInGameUI()
    local Rect = RectTransform()
    local Color = Vector4(0,1,0,1)

    -- 화면 사이즈 및 뷰포트 오프셋 가져오기
    local ScreenSize = GetScreenSize()
    local ViewportOffset = GetViewportOffset()
    local HorizontalPadding = 8  -- 좌우 패딩
    local EdgeMargin = 50  -- 화면 가장자리 여백
    local BoxHeight = 50
    local TextHeight = 30
    local VerticalCenter = (BoxHeight - TextHeight) / 2  -- 세로 중앙 정렬

    local RemainHeight = (-39 - GetPlayer().Location.Z) * -1;
    if RemainHeight < 0 then
        RemainHeight = 0
    end
    local HeightText = "남은 높이: "..string.format("%.1f", RemainHeight).."m"

    -- 남은 높이 배경
    local LeftBoxWidth = 230
    Rect.Pos = Vector2D(ViewportOffset.X + EdgeMargin, ViewportOffset.Y + 10)
    Rect.Size = Vector2D(LeftBoxWidth, BoxHeight)
    Rect.Pivot = Vector2D(0, 0)
    Rect.ZOrder = 0
    DrawUISprite(Rect, "Data/UI/BlackBox.png", 0.6)

    -- 남은 높이 텍스트 (세로 중앙 정렬)
    Rect.Pos = Vector2D(ViewportOffset.X + EdgeMargin + HorizontalPadding, ViewportOffset.Y + 10 + VerticalCenter)
    Rect.Size = Vector2D(LeftBoxWidth - HorizontalPadding * 2, TextHeight)
    Rect.ZOrder = 1
    DrawUIText(Rect, HeightText, Color, 30, "THEFACESHOP INKLIPQUID")

    -- 플레이 시간 배경
    local RightBoxWidth = 260
    Rect.Pos = Vector2D(ViewportOffset.X + ScreenSize.X - RightBoxWidth - EdgeMargin, ViewportOffset.Y + 10)
    Rect.Size = Vector2D(RightBoxWidth, BoxHeight)
    Rect.ZOrder = 0
    DrawUISprite(Rect, "Data/UI/BlackBox.png", 0.6)

    -- 플레이 시간 텍스트 (세로 중앙 정렬)
    Rect.Pos = Vector2D(ViewportOffset.X + ScreenSize.X - RightBoxWidth - EdgeMargin + HorizontalPadding, ViewportOffset.Y + 10 + VerticalCenter)
    Rect.Size = Vector2D(RightBoxWidth - HorizontalPadding * 2, TextHeight)
    Rect.ZOrder = 1
    Color = Vector4(1,0.5,0.5,1)
    DrawUIText(Rect, "플레이 시간: "..string.format("%.1f", PlayTime).."초", Color, 30, "THEFACESHOP INKLIPQUID")

    RenderCredits()
end

-- 제작자 표시 함수 (모든 화면에 공통 표시)
function RenderCredits()
    local Rect = RectTransform()
    local GrayColor = Vector4(0.7, 0.7, 0.7, 1)  -- 회색

    -- 배경 박스
    local AnchorMin = Vector2D(0.02, 0.93)  -- 좌하단에서 2% 여백 (Y: 0=상단, 1=하단)
    local AnchorMax = Vector2D(0.26, 0.98)  -- 24% 너비, 5% 높이
    Rect = FRectTransform.CreateAnchorRange(AnchorMin, AnchorMax)
    Rect.ZOrder = 100
    DrawUISprite(Rect, "Data/UI/BlackBox.png", 0.7)

    -- 제작자 텍스트
    AnchorMin = Vector2D(0.025, 0.935)  -- 박스 안쪽 패딩
    AnchorMax = Vector2D(0.255, 0.975)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin, AnchorMax)
    Rect.ZOrder = 101
    DrawUIText(Rect, "제작자: 김상천, 김진철, 김호민, 김희준", GrayColor, 16, "Pretendard")
end

-- 랜덤 사망 문구 목록
DeathMessages = {
    "중력 체험 완료",
    "지구와 강렬한 포옹",
    "뉴턴이 당신을 기억합니다",
    "무릎이 남아나질 않겠군요",
    "날개는 챙기셨나요?",
    "착지 점수: 0점",
    "낙하산 미착용"
}

CurrentDeathMessage = ""
DeathCount = 0  -- 사망 횟수 카운터

-- 사망 UI 출력
function RenderDeathUI()
    local Rect = RectTransform()
    local Color = Vector4(1,0,0,1)

    -- 낙사 텍스트 배경 박스
    local AnchorMin = Vector2D(0.2, 0.47)
    local AnchorMax = Vector2D(0.8, 0.58)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin, AnchorMax)
    Rect.ZOrder = 0
    DrawUISprite(Rect, "Data/UI/BlackBox.png", 0.5)

    -- 낙사 텍스트
    AnchorMin = Vector2D(0.2, 0.47)
    AnchorMax = Vector2D(0.8, 0.58)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin,AnchorMax)
    Rect.ZOrder = 1;
    DrawUIText(Rect, CurrentDeathMessage, Color, 50, "THEFACESHOP INKLIPQUID")

    -- 재시작 안내 배경 박스
    AnchorMin = Vector2D(0, 0.25)
    AnchorMax = Vector2D(1, 0.35)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin, AnchorMax)
    Rect.ZOrder = 0
    DrawUISprite(Rect, "Data/UI/BlackBox.png", 0.5)

    -- 재시작 안내 텍스트
    AnchorMin = Vector2D(0,0.2)
    AnchorMax = Vector2D(1,0.4)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin,AnchorMax)
    Rect.ZOrder = 1;
    local NeonPink = Vector4(1.0, 0.1, 0.7, 1.0)  -- 네온 핑크
    DrawUIText(Rect, "PRESS E TO RESTART", NeonPink, 40, "Platinum Sign")

    RenderCredits()
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

    -- 클리어 텍스트 배경 박스
    AnchorMin = Vector2D(0.25, 0.35)
    AnchorMax = Vector2D(0.75, 0.65)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin, AnchorMax)
    Rect.ZOrder = 0
    DrawUISprite(Rect, "Data/UI/BlackBox.png", 0.3)

    -- 클리어 텍스트
    AnchorMin = Vector2D(0,0)
    AnchorMax = Vector2D(1,1)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin,AnchorMax)
    Rect.ZOrder = 1;
    DrawUIText(Rect, "클리어!\n"..string.format("%.1f", PlayTime).."초", Color, 80, "THEFACESHOP INKLIPQUID")

    RenderCredits()
end
