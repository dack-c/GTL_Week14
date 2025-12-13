if not _G.GlobalConfig then
    _G.GlobalConfig = {
        GameState = "Init",
        bIsGameClear = false,
        bIsPlayerDeath = false,
        bFirstClearDone = false,

        CurrentScore = 0,
        TotalScore = 0,
        CurrentState = "None",
        PreviousState = "None",
        ComboCount = 0,
        LastScoreTime = 0,
        LastStateTransitionTime = 0,
        LastTransitionedState = "None",
        StateHistory = {},
        FallingStartTime = 0,
        CurrentFallingScore = 0,

        MotionBlurActive = false,
        MotionBlurIntensity = 0.0,
        MotionBlurStartTime = 0,
        MotionBlurDuration = 0.3
    }
end
GlobalConfig = _G.GlobalConfig

-- State별 점수 테이블 (Jump 제외, Falling은 시간 비례)
local StateScores = {
    ["Vault"] = 25,
    ["Slide"] = 15,
    ["Sliding Start"] = 10,
    ["Roll"] = 20,
    ["Climb"] = 30
}

local ComboMultipliers = {
    [1] = 1.0,
    [2] = 1.2,
    [3] = 1.5,
    [4] = 2.0,
    [5] = 2.5
}

local MotionBlurIntensities = {
    ["Vault"] = 1.2,
    ["Slide"] = 1.0,
    ["Sliding Start"] = 0.8,
    ["Sliding"] = 0.9,
    ["Roll"] = 1.1,
    ["Dash"] = 1.5
}

local PlayerMaxSpeed = 3500.0
local SpeedBlurThreshold = PlayerMaxSpeed * 0.70
local SpeedBlurMaxSpeed = PlayerMaxSpeed
local SpeedBlurMaxIntensity = 1.2

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

local TotalPlayTime = 0
local CurrentAttemptTime = 0

function InitGame()
    -- TODO: 플레이어 생성
    CurrentAttemptTime = 0
    -- TotalPlayTime은 타이틀 복귀 시에만 리셋됨 (Clear state에서 E 누를 때)

    GlobalConfig.CurrentScore = 0
    GlobalConfig.CurrentState = "None"
    GlobalConfig.PreviousState = "None"
    GlobalConfig.ComboCount = 0
    GlobalConfig.LastScoreTime = 0
    GlobalConfig.StateHistory = {}
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
    if InputManager:IsKeyDown("P") then
        InitGame()
        GlobalConfig.GameState = "Init"
    end

    -- 히든 디버그: G 키로 골 근처 스폰
    if InputManager:IsKeyDown("G") then
        local GoalPos = Vector(20, 196, -25)
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
        GlobalConfig.bIsGameClear = false
        GlobalConfig.bIsPlayerDeath = false
        GlobalConfig.GameState = "Playing"
        SetPlayerInputEnabled(true)

    elseif GlobalConfig.GameState == "Playing" then
        if not GlobalConfig.bFirstClearDone then
            TotalPlayTime = TotalPlayTime + dt
        end
        CurrentAttemptTime = CurrentAttemptTime + dt

        UpdatePlayerStateAndScore(dt)

        -- [TEST] 상시 Motion Blur 테스트 (Q 키)
        if InputManager:IsKeyPressed("B") then
            local CameraManager = GetCameraManager()
            if CameraManager then
                CameraManager:StartMotionBlur(1.0)
                print("[DEBUG] Motion Blur triggered manually (Intensity: 1.0)")
            end
        end

        if GlobalConfig.MotionBlurActive then
            local Player = GetPlayer()
            local CurrentStateBlurIntensity = nil

            if Player then
                local SkeletalMesh = GetComponent(Player, "USkeletalMeshComponent")
                if SkeletalMesh then
                    local AnimInstance = GetAnimInstanceOfSkeletal(SkeletalMesh)
                    if AnimInstance then
                        local CurrentState = AnimInstance:GetCurrentStateName()
                        CurrentStateBlurIntensity = MotionBlurIntensities[CurrentState]
                    end
                end
            end

            if CurrentStateBlurIntensity and CurrentStateBlurIntensity > 0 then
                -- 지속 상태 (예: Sliding): 블러 유지
                local CameraManager = GetCameraManager()
                if CameraManager then
                    CameraManager:StartMotionBlur(CurrentStateBlurIntensity)
                end
            else
                local BlurElapsed = TotalPlayTime - GlobalConfig.MotionBlurStartTime
                if BlurElapsed < GlobalConfig.MotionBlurDuration then
                    -- 페이드아웃 중: 강도를 선형 감소
                    local Progress = BlurElapsed / GlobalConfig.MotionBlurDuration
                    local CurrentIntensity = GlobalConfig.MotionBlurIntensity * (1.0 - Progress)

                    local CameraManager = GetCameraManager()
                    if CameraManager then
                        CameraManager:StartMotionBlur(CurrentIntensity)
                    end
                else
                    GlobalConfig.MotionBlurActive = false
                    GlobalConfig.MotionBlurIntensity = 0.0

                    local CameraManager = GetCameraManager()
                    if CameraManager then
                        CameraManager:StartMotionBlur(0.0)
                    end
                end
            end
        -- State 기반 블러가 없을 때: 속도 기반 블러 체크
        else
            local Player = GetPlayer()
            if Player then
                local Vel = Player.Velocity
                -- 수평 속도 계산 (XY 평면, Z 제외)
                local HorizontalSpeed = math.sqrt(Vel.X * Vel.X + Vel.Y * Vel.Y)

                if HorizontalSpeed > SpeedBlurThreshold then
                    local SpeedRatio = math.min((HorizontalSpeed - SpeedBlurThreshold) / (SpeedBlurMaxSpeed - SpeedBlurThreshold), 1.0)
                    local SpeedBasedIntensity = SpeedRatio * SpeedBlurMaxIntensity

                    local CameraManager = GetCameraManager()
                    if CameraManager then
                        CameraManager:StartMotionBlur(SpeedBasedIntensity)
                    end
                else
                    local CameraManager = GetCameraManager()
                    if CameraManager then
                        CameraManager:StartMotionBlur(0.0)
                    end
                end
            end
        end

        RenderInGameUI()

        -- 클리어 체크가 먼저 (우선순위 높음)
        if GlobalConfig.bIsGameClear == true then
            GlobalConfig.GameState = "Clear"
            GlobalConfig.bFirstClearDone = true
            GetComponent(GetPlayer(), "USpringArmComponent").CameraLagSpeed = 0
            SetPlayerInputEnabled(false)
        elseif GlobalConfig.bIsPlayerDeath == true then
            GlobalConfig.GameState = "Death"
            GetComponent(GetPlayer(), "USkeletalMeshComponent"):SetRagdoll(true)
            GetComponent(GetPlayer(), "USpringArmComponent").CameraLagSpeed = 0
            PlaySound2DOneShotByFile("Data/Audio/Scream.wav")

            local RandomIndex = RandomInt(1, #DeathMessages)
            CurrentDeathMessage = DeathMessages[RandomIndex]
            SetPlayerInputEnabled(false)
        -- 아직 클리어/사망 안 했을 때만 낙사 판정
        else
            local PlayerPos = GetPlayer().Location
            local GoalHeight = -34.777500
            local GoalPosX = -31.036255
            local GoalPosY = 190.940994
            local GoalRadius = 35.0

            local DistanceToGoal = math.sqrt((PlayerPos.X - GoalPosX)^2 + (PlayerPos.Y - GoalPosY)^2)
            local bIsOutsideGoalArea = DistanceToGoal > GoalRadius

            if bIsOutsideGoalArea and PlayerPos.Z < GoalHeight then
                GlobalConfig.bIsPlayerDeath = true
            end
        end

    elseif GlobalConfig.GameState == "Death" then
        RenderDeathUI()

        if InputManager:IsKeyDown("E") then
            InitGame()
            GlobalConfig.GameState = "Init"
            CurrentDeathMessage = ""
        end

    elseif GlobalConfig.GameState == "Clear" then
        RenderClearUI()

        if InputManager:IsKeyDown("E") then
            GlobalConfig.GameState = "Init"
            GlobalConfig.bFirstClearDone = false
            TotalPlayTime = 0
            CurrentAttemptTime = 0
            CurrentDeathMessage = ""

            GlobalConfig.CurrentScore = 0
            GlobalConfig.TotalScore = 0
            GlobalConfig.CurrentState = "None"
            GlobalConfig.PreviousState = "None"
            GlobalConfig.ComboCount = 0
            GlobalConfig.LastScoreTime = 0
            GlobalConfig.StateHistory = {}
            GlobalConfig.FallingStartTime = 0
            GlobalConfig.CurrentFallingScore = 0
        end

    end
end

function UpdatePlayerStateAndScore(dt)
    local Player = GetPlayer()
    if not Player then
        return
    end

    local SkeletalMesh = GetComponent(Player, "USkeletalMeshComponent")
    if not SkeletalMesh then
        return
    end

    local AnimInstance = GetAnimInstanceOfSkeletal(SkeletalMesh)
    if not AnimInstance then
        return
    end

    local CurrentState = AnimInstance:GetCurrentStateName()

    if CurrentState ~= GlobalConfig.CurrentState then
        -- 중복 점수 가산 방지: 같은 이전 State에서 0.1초 내에 다시 점수를 주지 않음
        local TimeSinceLastScore = TotalPlayTime - GlobalConfig.LastScoreTime
        local WillScoreForState = GlobalConfig.CurrentState

        -- Falling Idle은 Long Falling으로 기록됨
        if GlobalConfig.CurrentState == "Falling Idle" then
            local FallingDuration = TotalPlayTime - GlobalConfig.FallingStartTime
            if FallingDuration > 1.0 then
                WillScoreForState = "Long Falling"
            else
                WillScoreForState = nil
            end
        elseif not StateScores[GlobalConfig.CurrentState] then
            WillScoreForState = nil
        end

        if WillScoreForState and GlobalConfig.LastTransitionedState == WillScoreForState and TimeSinceLastScore < 0.1 then
            return
        end

        if GlobalConfig.CurrentState == "Falling Idle" then
            local FallingDuration = TotalPlayTime - GlobalConfig.FallingStartTime
            if FallingDuration > 1.0 then
                local ExtraTime = FallingDuration - 1.0
                local BaseScore = math.floor(ExtraTime * 10)  -- 0.1초당 1점

                local TimeSinceLastScore = TotalPlayTime - GlobalConfig.LastScoreTime
                if TimeSinceLastScore <= 3.0 and GlobalConfig.ComboCount > 0 then
                    GlobalConfig.ComboCount = GlobalConfig.ComboCount + 1
                else
                    GlobalConfig.ComboCount = 1
                end

                local ComboLevel = math.min(GlobalConfig.ComboCount, 5)
                local Multiplier = ComboMultipliers[ComboLevel]
                local FinalScore = math.floor(BaseScore * Multiplier)

                GlobalConfig.CurrentScore = GlobalConfig.CurrentScore + FinalScore
                GlobalConfig.LastScoreTime = TotalPlayTime
                GlobalConfig.LastTransitionedState = "Long Falling"

                table.insert(GlobalConfig.StateHistory, 1, {
                    StateName = "Long Falling",
                    Score = FinalScore,
                    Time = TotalPlayTime
                })

                if #GlobalConfig.StateHistory > 3 then
                    table.remove(GlobalConfig.StateHistory)
                end
            end
        end

        if GlobalConfig.CurrentState ~= "None" and StateScores[GlobalConfig.CurrentState] then
            local BaseScore = StateScores[GlobalConfig.CurrentState]

            -- 콤보 체크 (마지막 점수 획득 후 3초 이내면 콤보 유지)
            local TimeSinceLastScore = TotalPlayTime - GlobalConfig.LastScoreTime
            if TimeSinceLastScore <= 3.0 and GlobalConfig.ComboCount > 0 then
                GlobalConfig.ComboCount = GlobalConfig.ComboCount + 1
            else
                GlobalConfig.ComboCount = 1
            end

            local ComboLevel = math.min(GlobalConfig.ComboCount, 5)
            local Multiplier = ComboMultipliers[ComboLevel]
            local FinalScore = math.floor(BaseScore * Multiplier)

            GlobalConfig.CurrentScore = GlobalConfig.CurrentScore + FinalScore
            GlobalConfig.LastScoreTime = TotalPlayTime
            GlobalConfig.LastTransitionedState = GlobalConfig.CurrentState

            -- State 히스토리에 추가 (최대 3개 유지)
            table.insert(GlobalConfig.StateHistory, 1, {
                StateName = GlobalConfig.CurrentState,
                Score = FinalScore,
                Time = TotalPlayTime
            })

            if #GlobalConfig.StateHistory > 3 then
                table.remove(GlobalConfig.StateHistory)
            end
        end

        if CurrentState == "Falling Idle" then
            GlobalConfig.FallingStartTime = TotalPlayTime
        end

        -- 상태 업데이트 (모든 State로 업데이트하여 중복 점수 방지)
        GlobalConfig.PreviousState = GlobalConfig.CurrentState
        GlobalConfig.CurrentState = CurrentState

        local BlurIntensity = MotionBlurIntensities[CurrentState]
        if BlurIntensity and BlurIntensity > 0 then
            GlobalConfig.MotionBlurActive = true
            GlobalConfig.MotionBlurIntensity = BlurIntensity
            GlobalConfig.MotionBlurStartTime = TotalPlayTime

            local CameraManager = GetCameraManager()
            if CameraManager then
                CameraManager:StartMotionBlur(BlurIntensity)
            end
        end
    end

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

function RenderInitUI()
    local Rect = RectTransform()
    local Color = Vector4(1,1,1,1)
    local NeonCyan = Vector4(0.0, 1.0, 1.0, 1.0)

    local AnchorMin = Vector2D(0,0)
    local AnchorMax = Vector2D(1,1)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin,AnchorMax)
    DrawUISprite(Rect, "Data/UI/Main.png", 1.0)

    AnchorMin = Vector2D(0, 0.65)
    AnchorMax = Vector2D(1, 0.75)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin, AnchorMax)
    Rect.ZOrder = 0
    DrawUISprite(Rect, "Data/UI/BlackBox.png", 0.3)

    AnchorMin = Vector2D(0,0.6)
    AnchorMax = Vector2D(1,0.8)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin,AnchorMax)
    Rect.ZOrder = 1;
    DrawUIText(Rect, "PRESS Q TO START", NeonCyan, 40, "Platinum Sign")

    RenderCredits()
end

function RenderInGameUI()
    local Rect = RectTransform()
    local Color = Vector4(0,1,0,1)

    local ScreenSize = GetScreenSize()
    local ViewportOffset = GetViewportOffset()
    local HorizontalPadding = 8
    local EdgeMargin = 50
    local BoxHeight = 50
    local TextHeight = 30
    local VerticalCenter = (BoxHeight - TextHeight) / 2

    local RemainHeight = (-39 - GetPlayer().Location.Z) * -1;
    if RemainHeight < 0 then
        RemainHeight = 0
    end
    local HeightText = "남은 높이: "..string.format("%.1f", RemainHeight).."m"

    local LeftBoxWidth = 230
    Rect.Pos = Vector2D(ViewportOffset.X + EdgeMargin, ViewportOffset.Y + 10)
    Rect.Size = Vector2D(LeftBoxWidth, BoxHeight)
    Rect.Pivot = Vector2D(0, 0)
    Rect.ZOrder = 0
    DrawUISprite(Rect, "Data/UI/BlackBox.png", 0.6)

    Rect.Pos = Vector2D(ViewportOffset.X + EdgeMargin + HorizontalPadding, ViewportOffset.Y + 10 + VerticalCenter)
    Rect.Size = Vector2D(LeftBoxWidth - HorizontalPadding * 2, TextHeight)
    Rect.ZOrder = 1
    DrawUIText(Rect, HeightText, Color, 30, "THEFACESHOP INKLIPQUID")

    local RightBoxWidth = 260
    Rect.Pos = Vector2D(ViewportOffset.X + ScreenSize.X - RightBoxWidth - EdgeMargin, ViewportOffset.Y + 10)
    Rect.Size = Vector2D(RightBoxWidth, BoxHeight)
    Rect.ZOrder = 0
    DrawUISprite(Rect, "Data/UI/BlackBox.png", 0.6)

    Rect.Pos = Vector2D(ViewportOffset.X + ScreenSize.X - RightBoxWidth - EdgeMargin + HorizontalPadding, ViewportOffset.Y + 10 + VerticalCenter)
    Rect.Size = Vector2D(RightBoxWidth - HorizontalPadding * 2, TextHeight)
    Rect.ZOrder = 1
    Color = Vector4(1,0.8,0.5,1)
    DrawUIText(Rect, "총 시간: "..string.format("%.1f", TotalPlayTime).."초", Color, 30, "THEFACESHOP INKLIPQUID")

    Rect.Pos = Vector2D(ViewportOffset.X + ScreenSize.X - RightBoxWidth - EdgeMargin, ViewportOffset.Y + 10 + BoxHeight + 5)
    Rect.Size = Vector2D(RightBoxWidth, BoxHeight)
    Rect.ZOrder = 0
    DrawUISprite(Rect, "Data/UI/BlackBox.png", 0.6)

    Rect.Pos = Vector2D(ViewportOffset.X + ScreenSize.X - RightBoxWidth - EdgeMargin + HorizontalPadding, ViewportOffset.Y + 10 + BoxHeight + 5 + VerticalCenter)
    Rect.Size = Vector2D(RightBoxWidth - HorizontalPadding * 2, TextHeight)
    Rect.ZOrder = 1
    Color = Vector4(0.5,1,0.5,1)
    DrawUIText(Rect, "시도 시간: "..string.format("%.1f", CurrentAttemptTime).."초", Color, 30, "THEFACESHOP INKLIPQUID")

    RenderPlayerScoreUI()
    RenderCredits()
end

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

    AnchorMin = Vector2D(0.60, BaseY)
    AnchorMax = Vector2D(0.67, BaseY + 0.03)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin, AnchorMax)
    Rect.ZOrder = 11
    DrawUIText(Rect, string.format("%d", GlobalConfig.CurrentScore), YellowColor, 28, "Freesentation 8 ExtraBold")

    -- 현재 State 표시 (StateHistory[1] 사용, 중복 방지)
    if #GlobalConfig.StateHistory > 0 then
        local CurrentStateItem = GlobalConfig.StateHistory[1]
        AnchorMin = Vector2D(0.67, BaseY - 0.003)
        AnchorMax = Vector2D(0.80, BaseY + 0.036)
        Rect = FRectTransform.CreateAnchorRange(AnchorMin, AnchorMax)

        Rect.ZOrder = 10
        DrawUIRect(Rect, YellowColor, 2.0)

        Rect.ZOrder = 11
        DrawUIText(Rect, string.format("%s", CurrentStateItem.StateName), WhiteColor, 24, "Freesentation 8 ExtraBold")
    end

    -- State 히스토리 큐 렌더링 (세로로 아래에 쌓임, [2], [3]만 표시)
    local HistoryCount = #GlobalConfig.StateHistory
    local LastHistoryY = BaseY

    if HistoryCount > 1 then
        -- 히스토리 항목 렌더링 (순서: 2→3, [1]은 현재 State로 이미 표시됨)
        for i = 2, HistoryCount do
            local HistoryItem = GlobalConfig.StateHistory[i]

            local HistoryIndex = i - 1
            local YOffset = BaseY - 0.005 - (HistoryIndex * 0.03)
            LastHistoryY = YOffset

            local Alpha = 1.0 - ((HistoryIndex - 1) * 0.325)
            local FontSize = 22 - (HistoryIndex * 2)

            AnchorMin = Vector2D(0.67, YOffset)
            AnchorMax = Vector2D(0.80, YOffset + 0.03)
            Rect = FRectTransform.CreateAnchorRange(AnchorMin, AnchorMax)
            Rect.ZOrder = 11

            local FadedColor = Vector4(1, 1, 1, Alpha)
            local DisplayText = string.format("%s", HistoryItem.StateName)
            DrawUIText(Rect, DisplayText, FadedColor, FontSize, "Freesentation 8 ExtraBold")
        end
    end

    if GlobalConfig.CurrentFallingScore > 0 then
        local ScoreY = BaseY + 0.04
        AnchorMin = Vector2D(0.74, ScoreY)
        AnchorMax = Vector2D(0.80, ScoreY + 0.025)
        Rect = FRectTransform.CreateAnchorRange(AnchorMin, AnchorMax)
        Rect.ZOrder = 11
        local GreenColor = Vector4(0, 1, 0, 1)
        DrawUIText(Rect, string.format("+%d", GlobalConfig.CurrentFallingScore), GreenColor, 18, "Freesentation 8 ExtraBold")
    end
end

function RenderCredits()
    local Rect = RectTransform()
    local GrayColor = Vector4(0.7, 0.7, 0.7, 1)

    local AnchorMin = Vector2D(0.02, 0.93)
    local AnchorMax = Vector2D(0.26, 0.98)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin, AnchorMax)
    Rect.ZOrder = 100
    DrawUISprite(Rect, "Data/UI/BlackBox.png", 0.7)

    AnchorMin = Vector2D(0.025, 0.935)
    AnchorMax = Vector2D(0.255, 0.975)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin, AnchorMax)
    Rect.ZOrder = 101
    DrawUIText(Rect, "제작자: 김상천, 김진철, 김호민, 김희준", GrayColor, 16, "Pretendard")
end

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
DeathCount = 0

function RenderDeathUI()
    local Rect = RectTransform()
    local Color = Vector4(1,0,0,1)

    local AnchorMin = Vector2D(0.2, 0.47)
    local AnchorMax = Vector2D(0.8, 0.58)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin, AnchorMax)
    Rect.ZOrder = 0
    DrawUISprite(Rect, "Data/UI/BlackBox.png", 0.5)

    AnchorMin = Vector2D(0.2, 0.47)
    AnchorMax = Vector2D(0.8, 0.58)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin,AnchorMax)
    Rect.ZOrder = 1;
    DrawUIText(Rect, CurrentDeathMessage, Color, 50, "THEFACESHOP INKLIPQUID")

    AnchorMin = Vector2D(0, 0.25)
    AnchorMax = Vector2D(1, 0.35)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin, AnchorMax)
    Rect.ZOrder = 0
    DrawUISprite(Rect, "Data/UI/BlackBox.png", 0.5)

    AnchorMin = Vector2D(0,0.2)
    AnchorMax = Vector2D(1,0.4)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin,AnchorMax)
    Rect.ZOrder = 1;
    local NeonPink = Vector4(1.0, 0.1, 0.7, 1.0)
    DrawUIText(Rect, "PRESS E TO RESTART", NeonPink, 40, "Platinum Sign")

    RenderCredits()
end

function RenderClearUI()
    local Rect = RectTransform()
    local Color = Vector4(0,1,1,1)
    local NeonCyan = Vector4(0.0, 1.0, 1.0, 1.0)

    local AnchorMin = Vector2D(0.25, 0.28)
    local AnchorMax = Vector2D(0.75, 0.62)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin, AnchorMax)
    Rect.ZOrder = 0
    DrawUISprite(Rect, "Data/UI/BlackBox.png", 0.5)

    AnchorMin = Vector2D(0, 0.3)
    AnchorMax = Vector2D(1, 0.42)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin, AnchorMax)
    Rect.ZOrder = 1
    DrawUIText(Rect, "클리어!", Color, 80, "THEFACESHOP INKLIPQUID")

    AnchorMin = Vector2D(0, 0.43)
    AnchorMax = Vector2D(1, 0.49)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin, AnchorMax)
    Rect.ZOrder = 1
    local OrangeColor = Vector4(1, 0.8, 0.5, 1)
    DrawUIText(Rect, "총 시간: "..string.format("%.1f", TotalPlayTime).."초", OrangeColor, 34, "THEFACESHOP INKLIPQUID")

    AnchorMin = Vector2D(0, 0.49)
    AnchorMax = Vector2D(1, 0.55)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin, AnchorMax)
    Rect.ZOrder = 1
    local GreenColor = Vector4(0.5, 1, 0.5, 1)
    DrawUIText(Rect, "시도 시간: "..string.format("%.1f", CurrentAttemptTime).."초", GreenColor, 34, "THEFACESHOP INKLIPQUID")

    AnchorMin = Vector2D(0, 0.55)
    AnchorMax = Vector2D(1, 0.61)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin, AnchorMax)
    Rect.ZOrder = 1
    local YellowColor = Vector4(1, 1, 0, 1)
    DrawUIText(Rect, "SCORE: "..string.format("%d", GlobalConfig.CurrentScore), YellowColor, 34, "THEFACESHOP INKLIPQUID")

    AnchorMin = Vector2D(0, 0.63)
    AnchorMax = Vector2D(1, 0.78)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin, AnchorMax)
    Rect.ZOrder = 0
    DrawUISprite(Rect, "Data/UI/BlackBox.png", 0.5)

    AnchorMin = Vector2D(0, 0.63)
    AnchorMax = Vector2D(1, 0.78)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin, AnchorMax)
    Rect.ZOrder = 1
    DrawUIText(Rect, "PRESS E\nTO RETURN TO TITLE", NeonCyan, 32, "Platinum Sign")

    RenderCredits()
end
