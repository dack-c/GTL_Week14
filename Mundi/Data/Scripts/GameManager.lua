-- 전역 GlobalConfig 테이블을 전역에 초기화
if not _G.GlobalConfig then
    _G.GlobalConfig = {
        GameState = "Init",
        bIsGameClear = false,
        bIsPlayerDeath = false,
        bFirstClearDone = false,  -- 첫 클리어 여부

        -- Score System
        CurrentScore = 0,           -- 현재 시도 점수
        TotalScore = 0,             -- 누적 최고 점수
        CurrentState = "None",      -- 현재 동작 상태 (내부 추적용)
        DisplayState = "None",      -- UI 표시용 State (점수 준 것만)
        PreviousState = "None",     -- 이전 동작 상태
        ComboCount = 0,             -- 연속 콤보 카운트
        LastScoreTime = 0,          -- 마지막 점수 획득 시간
        LastStateTransitionTime = 0, -- 마지막 State 전환 시간 (중복 방지)
        LastTransitionedState = "None", -- 마지막 전환된 State (중복 방지)
        StateHistory = {},          -- State 히스토리 큐 (최대 3개)
        FallingStartTime = 0,       -- Falling 시작 시간
        CurrentFallingScore = 0     -- 현재 Falling 중 실시간 점수
    }
end
GlobalConfig = _G.GlobalConfig  -- 로컬 별칭

-- State별 점수 테이블 (Jump 제외, Falling은 시간 비례)
local StateScores = {
    ["Vault"] = 25,
    ["Slide"] = 15,
    ["Sliding Start"] = 10,
    ["Roll"] = 20,
    ["Climb"] = 30
}

-- 콤보 배율 (연속 동작 시)
local ComboMultipliers = {
    [1] = 1.0,   -- 1콤보
    [2] = 1.2,   -- 2콤보: 1.2배
    [3] = 1.5,   -- 3콤보: 1.5배
    [4] = 2.0,   -- 4콤보: 2.0배
    [5] = 2.5    -- 5콤보 이상: 2.5배
}

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

local TotalPlayTime = 0        -- 총 플레이 타임 (첫 클리어까지 누적)
local CurrentAttemptTime = 0   -- 현재 시도 시간 (사망 시 리셋)

-- 게임 상태 초기화
function InitGame()
    -- TODO: 플레이어 생성
    CurrentAttemptTime = 0  -- 현재 시도만 리셋
    -- TotalPlayTime은 타이틀 복귀 시에만 리셋됨 (Clear state에서 E 누를 때)

    -- Score 시스템 초기화
    GlobalConfig.CurrentScore = 0
    GlobalConfig.CurrentState = "None"
    GlobalConfig.DisplayState = "None"
    GlobalConfig.PreviousState = "None"
    GlobalConfig.ComboCount = 0
    GlobalConfig.LastScoreTime = 0
    GlobalConfig.StateHistory = {}  -- 히스토리 클리어
    GlobalConfig.FallingStartTime = 0
    GlobalConfig.CurrentFallingScore = 0

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
        -- 첫 클리어 전까지 총 플레이 타임 누적
        if not GlobalConfig.bFirstClearDone then
            TotalPlayTime = TotalPlayTime + dt
        end
        CurrentAttemptTime = CurrentAttemptTime + dt

        -- 플레이어 애니메이션 상태 감지 및 점수 계산
        UpdatePlayerStateAndScore(dt)

        RenderInGameUI()

        -- 클리어 체크가 먼저 (우선순위 높음)
        if GlobalConfig.bIsGameClear == true then
            GlobalConfig.GameState = "Clear"
            GlobalConfig.bFirstClearDone = true  -- 첫 클리어 완료 표시
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
            InitGame()  -- 사망 시 게임 리셋 (스코어 포함)
            GlobalConfig.GameState = "Init"
            CurrentDeathMessage = ""  -- 사망 메시지 리셋
        end

    elseif GlobalConfig.GameState == "Clear" then
        -- 클리어 타임 표시
        RenderClearUI()

        if InputManager:IsKeyDown("E") then
            GlobalConfig.GameState = "Init"
            GlobalConfig.bFirstClearDone = false  -- 첫 클리어 플래그 리셋
            TotalPlayTime = 0  -- 총 시간 리셋
            CurrentAttemptTime = 0  -- 시도 시간 리셋
            CurrentDeathMessage = ""  -- 사망 메시지 리셋

            -- 스코어 시스템 완전 리셋
            GlobalConfig.CurrentScore = 0
            GlobalConfig.TotalScore = 0
            GlobalConfig.CurrentState = "None"
            GlobalConfig.DisplayState = "None"
            GlobalConfig.PreviousState = "None"
            GlobalConfig.ComboCount = 0
            GlobalConfig.LastScoreTime = 0
            GlobalConfig.StateHistory = {}  -- 히스토리 클리어
            GlobalConfig.FallingStartTime = 0
            GlobalConfig.CurrentFallingScore = 0
        end

    end
end

-- 플레이어 애니메이션 상태 감지 및 점수 계산
function UpdatePlayerStateAndScore(dt)
    local Player = GetPlayer()
    if not Player then
        return
    end

    -- 플레이어의 스켈레탈 메쉬와 애니메이션 인스턴스 가져오기
    local SkeletalMesh = GetComponent(Player, "USkeletalMeshComponent")
    if not SkeletalMesh then
        return
    end

    local AnimInstance = GetAnimInstanceOfSkeletal(SkeletalMesh)
    if not AnimInstance then
        return
    end

    -- 현재 애니메이션 상태 이름 가져오기
    local CurrentState = AnimInstance:GetCurrentStateName()

    -- 상태가 변경되었는지 체크
    if CurrentState ~= GlobalConfig.CurrentState then
        -- 중복 점수 가산 방지: 같은 이전 State에서 0.1초 내에 다시 점수를 주지 않음
        local TimeSinceLastScore = TotalPlayTime - GlobalConfig.LastScoreTime
        local WillScoreForState = GlobalConfig.CurrentState  -- 점수를 받을 State (이전 State)

        -- Falling Idle은 Long Falling으로 기록됨
        if GlobalConfig.CurrentState == "Falling Idle" then
            local FallingDuration = TotalPlayTime - GlobalConfig.FallingStartTime
            if FallingDuration > 1.0 then
                WillScoreForState = "Long Falling"
            else
                WillScoreForState = nil  -- 1초 미만은 점수 없음
            end
        elseif not StateScores[GlobalConfig.CurrentState] then
            WillScoreForState = nil  -- StateScores에 없으면 점수 없음
        end

        -- 같은 State에 대해 0.1초 내에 중복 점수 방지
        if WillScoreForState and GlobalConfig.LastTransitionedState == WillScoreForState and TimeSinceLastScore < 0.1 then
            return  -- 중복 점수 무시
        end

        -- Falling Idle 종료 시 시간 비례 점수 계산
        if GlobalConfig.CurrentState == "Falling Idle" then
            local FallingDuration = TotalPlayTime - GlobalConfig.FallingStartTime
            if FallingDuration > 1.0 then  -- 1초 이후부터 점수
                local ExtraTime = FallingDuration - 1.0
                local BaseScore = math.floor(ExtraTime * 10)  -- 0.1초당 1점

                -- 콤보 체크
                local TimeSinceLastScore = TotalPlayTime - GlobalConfig.LastScoreTime
                if TimeSinceLastScore <= 3.0 and GlobalConfig.ComboCount > 0 then
                    GlobalConfig.ComboCount = GlobalConfig.ComboCount + 1
                else
                    GlobalConfig.ComboCount = 1
                end

                -- 콤보 배율 적용
                local ComboLevel = math.min(GlobalConfig.ComboCount, 5)
                local Multiplier = ComboMultipliers[ComboLevel]
                local FinalScore = math.floor(BaseScore * Multiplier)

                -- 점수 가산
                GlobalConfig.CurrentScore = GlobalConfig.CurrentScore + FinalScore
                GlobalConfig.LastScoreTime = TotalPlayTime
                GlobalConfig.LastTransitionedState = "Long Falling"  -- 중복 방지용

                -- State 히스토리에 추가
                table.insert(GlobalConfig.StateHistory, 1, {
                    StateName = "Long Falling",
                    Score = FinalScore,
                    Time = TotalPlayTime
                })

                if #GlobalConfig.StateHistory > 3 then
                    table.remove(GlobalConfig.StateHistory)
                end

                -- DisplayState를 Long Falling으로 업데이트 (UI 표시용)
                GlobalConfig.DisplayState = "Long Falling"
            end
        end

        -- 이전 상태가 점수 테이블에 있고, None이 아니면 점수 부여
        if GlobalConfig.CurrentState ~= "None" and StateScores[GlobalConfig.CurrentState] then
            local BaseScore = StateScores[GlobalConfig.CurrentState]

            -- 콤보 체크 (마지막 점수 획득 후 3초 이내면 콤보 유지)
            local TimeSinceLastScore = TotalPlayTime - GlobalConfig.LastScoreTime
            if TimeSinceLastScore <= 3.0 and GlobalConfig.ComboCount > 0 then
                GlobalConfig.ComboCount = GlobalConfig.ComboCount + 1
            else
                GlobalConfig.ComboCount = 1
            end

            -- 콤보 배율 적용 (최대 5콤보)
            local ComboLevel = math.min(GlobalConfig.ComboCount, 5)
            local Multiplier = ComboMultipliers[ComboLevel]
            local FinalScore = math.floor(BaseScore * Multiplier)

            -- 점수 가산
            GlobalConfig.CurrentScore = GlobalConfig.CurrentScore + FinalScore
            GlobalConfig.LastScoreTime = TotalPlayTime
            GlobalConfig.LastTransitionedState = GlobalConfig.CurrentState  -- 중복 방지용

            -- State 히스토리에 추가 (최대 3개 유지)
            table.insert(GlobalConfig.StateHistory, 1, {
                StateName = GlobalConfig.CurrentState,
                Score = FinalScore,
                Time = TotalPlayTime
            })

            -- 3개 초과 시 제일 오래된 것 제거
            if #GlobalConfig.StateHistory > 3 then
                table.remove(GlobalConfig.StateHistory)
            end

            -- DisplayState 업데이트 (UI 표시용)
            GlobalConfig.DisplayState = GlobalConfig.CurrentState
        end

        -- Falling Idle 시작 시 시간 기록
        if CurrentState == "Falling Idle" then
            GlobalConfig.FallingStartTime = TotalPlayTime
        end

        -- 상태 업데이트 (모든 State로 업데이트하여 중복 점수 방지)
        GlobalConfig.PreviousState = GlobalConfig.CurrentState
        GlobalConfig.CurrentState = CurrentState
    end

    -- 현재 Falling 중인 경우 실시간 점수 계산 (실제 애니메이션 State 체크)
    if CurrentState == "Falling Idle" then
        local FallingDuration = TotalPlayTime - GlobalConfig.FallingStartTime
        GlobalConfig.CurrentFallingScore = 0
        if FallingDuration > 1.0 then
            local ExtraTime = FallingDuration - 1.0
            GlobalConfig.CurrentFallingScore = math.floor(ExtraTime * 10)
        end
    else
        GlobalConfig.CurrentFallingScore = 0
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

    -- 총 플레이 시간 배경 (위쪽)
    local RightBoxWidth = 260
    Rect.Pos = Vector2D(ViewportOffset.X + ScreenSize.X - RightBoxWidth - EdgeMargin, ViewportOffset.Y + 10)
    Rect.Size = Vector2D(RightBoxWidth, BoxHeight)
    Rect.ZOrder = 0
    DrawUISprite(Rect, "Data/UI/BlackBox.png", 0.6)

    -- 총 플레이 시간 텍스트 (세로 중앙 정렬)
    Rect.Pos = Vector2D(ViewportOffset.X + ScreenSize.X - RightBoxWidth - EdgeMargin + HorizontalPadding, ViewportOffset.Y + 10 + VerticalCenter)
    Rect.Size = Vector2D(RightBoxWidth - HorizontalPadding * 2, TextHeight)
    Rect.ZOrder = 1
    Color = Vector4(1,0.8,0.5,1)  -- 주황색
    DrawUIText(Rect, "총 시간: "..string.format("%.1f", TotalPlayTime).."초", Color, 30, "THEFACESHOP INKLIPQUID")

    -- 현재 시도 시간 배경 (아래쪽)
    Rect.Pos = Vector2D(ViewportOffset.X + ScreenSize.X - RightBoxWidth - EdgeMargin, ViewportOffset.Y + 10 + BoxHeight + 5)
    Rect.Size = Vector2D(RightBoxWidth, BoxHeight)
    Rect.ZOrder = 0
    DrawUISprite(Rect, "Data/UI/BlackBox.png", 0.6)

    -- 현재 시도 시간 텍스트 (세로 중앙 정렬)
    Rect.Pos = Vector2D(ViewportOffset.X + ScreenSize.X - RightBoxWidth - EdgeMargin + HorizontalPadding, ViewportOffset.Y + 10 + BoxHeight + 5 + VerticalCenter)
    Rect.Size = Vector2D(RightBoxWidth - HorizontalPadding * 2, TextHeight)
    Rect.ZOrder = 1
    Color = Vector4(0.5,1,0.5,1)  -- 연두색
    DrawUIText(Rect, "시도 시간: "..string.format("%.1f", CurrentAttemptTime).."초", Color, 30, "THEFACESHOP INKLIPQUID")

    -- 플레이어 옆에 Score 표시
    RenderPlayerScoreUI()

    RenderCredits()
end

-- 플레이어 옆에 Score UI 렌더링 (화면 우측, Anchor 비율 기반)
function RenderPlayerScoreUI()
    -- 시네마틱 모드에서는 Score UI 숨김
    if _G.CinematicState and _G.CinematicState.bIsActive then
        return
    end

    local Player = GetPlayer()
    if not Player then
        return
    end

    -- 0점일 때는 Score UI 표시하지 않음
    if GlobalConfig.CurrentScore == 0 then
        return
    end

    local Rect = RectTransform()
    local AnchorMin, AnchorMax
    local YellowColor = Vector4(1, 1, 0, 1)
    local WhiteColor = Vector4(1, 1, 1, 1)
    local BaseY = 0.45

    -- 점수만 표시 (노란색, 왼쪽)
    AnchorMin = Vector2D(0.60, BaseY)
    AnchorMax = Vector2D(0.67, BaseY + 0.03)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin, AnchorMax)
    Rect.ZOrder = 11
    DrawUIText(Rect, string.format("%d", GlobalConfig.CurrentScore), YellowColor, 28, "Freesentation 8 ExtraBold")

    -- 현재 State 표시 (흰색, 점수 옆 가로 배치)
    -- 점수 준 State만 표시 (빈 칸 방지)
    local DisplayStateName = GlobalConfig.DisplayState or "None"
    if DisplayStateName ~= "None" then
        AnchorMin = Vector2D(0.67, BaseY - 0.003)
        AnchorMax = Vector2D(0.80, BaseY + 0.036)
        Rect = FRectTransform.CreateAnchorRange(AnchorMin, AnchorMax)

        -- 테두리 박스 (노란색, 텍스트 뒤)
        Rect.ZOrder = 10
        DrawUIRect(Rect, YellowColor, 2.0)

        -- 텍스트 (테두리 위)
        Rect.ZOrder = 11
        DrawUIText(Rect, string.format("%s", DisplayStateName), WhiteColor, 24, "Freesentation 8 ExtraBold")
    end

    -- State 히스토리 큐 렌더링 (세로로 아래에 쌓임, 최근이 위)
    local HistoryCount = #GlobalConfig.StateHistory
    local LastHistoryY = BaseY  -- 마지막 히스토리 Y 위치 (기본은 현재 State 높이)

    if HistoryCount > 0 then
        -- 히스토리 항목 렌더링 (순서: 1→2→3, 최근이 현재 State 바로 위)
        for i = 1, HistoryCount do
            local HistoryItem = GlobalConfig.StateHistory[i]

            -- Y 오프셋 계산 (박스 상단과 간격 유지 + 0.03씩 위로)
            local YOffset = BaseY - 0.005 - (i * 0.03)
            LastHistoryY = YOffset  -- 마지막 히스토리 Y 위치 기록

            -- 투명도: 최신(1.0) → 중간(0.675) → 오래됨(0.35)
            local Alpha = 1.0 - ((i - 1) * 0.325)

            -- 글씨 크기: 최신(20) → 중간(18) → 오래됨(16)
            local FontSize = 22 - (i * 2)

            -- Anchor 기반 렌더링 (현재 State와 같은 X 위치)
            AnchorMin = Vector2D(0.67, YOffset)
            AnchorMax = Vector2D(0.80, YOffset + 0.03)
            Rect = FRectTransform.CreateAnchorRange(AnchorMin, AnchorMax)
            Rect.ZOrder = 11

            local FadedColor = Vector4(1, 1, 1, Alpha)
            local DisplayText = string.format("%s", HistoryItem.StateName)
            DrawUIText(Rect, DisplayText, FadedColor, FontSize, "Freesentation 8 ExtraBold")
        end
    end

    -- Falling 중 실시간 점수 표시 (현재 State 아래, State Box 오른쪽 정렬)
    if GlobalConfig.CurrentFallingScore > 0 then
        local ScoreY = BaseY + 0.04  -- 현재 State 박스 아래 (박스 하단 0.036 + 간격)
        AnchorMin = Vector2D(0.74, ScoreY)  -- Right align (State Box 오른쪽 기준)
        AnchorMax = Vector2D(0.80, ScoreY + 0.025)
        Rect = FRectTransform.CreateAnchorRange(AnchorMin, AnchorMax)
        Rect.ZOrder = 11
        local GreenColor = Vector4(0, 1, 0, 1)
        DrawUIText(Rect, string.format("+%d", GlobalConfig.CurrentFallingScore), GreenColor, 18, "Freesentation 8 ExtraBold")
    end
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
    local NeonCyan = Vector4(0.0, 1.0, 1.0, 1.0)

    -- 클리어 텍스트 배경 박스 (클리어! + 타임 로그 + 스코어, 상단~중앙)
    local AnchorMin = Vector2D(0.25, 0.28)
    local AnchorMax = Vector2D(0.75, 0.62)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin, AnchorMax)
    Rect.ZOrder = 0
    DrawUISprite(Rect, "Data/UI/BlackBox.png", 0.5)

    -- 클리어 텍스트 (타이틀) - 박스 내 상단
    AnchorMin = Vector2D(0, 0.3)
    AnchorMax = Vector2D(1, 0.42)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin, AnchorMax)
    Rect.ZOrder = 1
    DrawUIText(Rect, "클리어!", Color, 80, "THEFACESHOP INKLIPQUID")

    -- 총 플레이 시간
    AnchorMin = Vector2D(0, 0.43)
    AnchorMax = Vector2D(1, 0.49)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin, AnchorMax)
    Rect.ZOrder = 1
    local OrangeColor = Vector4(1, 0.8, 0.5, 1)
    DrawUIText(Rect, "총 시간: "..string.format("%.1f", TotalPlayTime).."초", OrangeColor, 34, "THEFACESHOP INKLIPQUID")

    -- 현재 시도 시간
    AnchorMin = Vector2D(0, 0.49)
    AnchorMax = Vector2D(1, 0.55)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin, AnchorMax)
    Rect.ZOrder = 1
    local GreenColor = Vector4(0.5, 1, 0.5, 1)
    DrawUIText(Rect, "시도 시간: "..string.format("%.1f", CurrentAttemptTime).."초", GreenColor, 34, "THEFACESHOP INKLIPQUID")

    -- 최종 스코어
    AnchorMin = Vector2D(0, 0.55)
    AnchorMax = Vector2D(1, 0.61)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin, AnchorMax)
    Rect.ZOrder = 1
    local YellowColor = Vector4(1, 1, 0, 1)
    DrawUIText(Rect, "SCORE: "..string.format("%d", GlobalConfig.CurrentScore), YellowColor, 34, "THEFACESHOP INKLIPQUID")

    -- PRESS E TO RETURN TO TITLE 배경 박스 (하단)
    AnchorMin = Vector2D(0, 0.63)
    AnchorMax = Vector2D(1, 0.78)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin, AnchorMax)
    Rect.ZOrder = 0
    DrawUISprite(Rect, "Data/UI/BlackBox.png", 0.5)

    -- PRESS E TO RETURN TO TITLE 텍스트
    AnchorMin = Vector2D(0, 0.63)
    AnchorMax = Vector2D(1, 0.78)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin, AnchorMax)
    Rect.ZOrder = 1
    DrawUIText(Rect, "PRESS E\nTO RETURN TO TITLE", NeonCyan, 32, "Platinum Sign")

    RenderCredits()
end
