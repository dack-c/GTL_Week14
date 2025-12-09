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
    GetComponent(GetPlayer(), "USpringArmComponent").CameraLagSpeed = 0.05
    GetPlayer().Location = GetStartPosition()
    GetComponent(GetPlayer(), "UCharacterMovementComponent"):ResetVelocity()
end


CurVisibilty = true
function Tick(dt)

    -- 강제 리플레이
    if InputManager:IsKeyDown("P") then
        InitGame()
        GlobalConfig.GameState = "Init"
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

    elseif GlobalConfig.GameState == "Playing" then
        PlayTime = PlayTime + dt
        RenderInGameUI()

        -- 클리어
        if GlobalConfig.bIsGameClear == true then
            GlobalConfig.GameState = "Clear"
            GetComponent(GetPlayer(), "USpringArmComponent").CameraLagSpeed = 0
        -- 사망
        elseif GlobalConfig.bIsPlayerDeath == true then
            GlobalConfig.GameState = "Death"
            GetComponent(GetPlayer(), "USpringArmComponent").CameraLagSpeed = 0
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

    local AnchorMin = Vector2D(0,0)
    local AnchorMax = Vector2D(1,1)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin,AnchorMax)
    DrawUISprite(Rect, "Data/UI/Main.png", 1.0)

    -- Rect.Pos = Vector2D(0.0,0.0)
    -- Rect.Size = Vector2D(1264.0,848.0)
    -- Rect.Pivot = Vector2D(0.0,0.0)
    -- Rect.ZOrder = 0;
    -- DrawUISprite(Rect, "Data/UI/Main.png", 1.0)
    
    AnchorMin = Vector2D(0,0.8)
    AnchorMax = Vector2D(1,1)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin,AnchorMax)
    Rect.ZOrder = 1;
    DrawUIText(Rect, "Press [Q] To Start", Color, 60)
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

    DrawUIText(Rect, "남은 높이: "..string.format("%.1f", RemainHeight).."m", Color, 30)

    
    AnchorMin = Vector2D(0.7,0)
    AnchorMax = Vector2D(1,0.2)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin,AnchorMax)
    Color = Vector4(1,0.5,0.5,1)
    DrawUIText(Rect, "플레이 시간: "..string.format("%.1f", PlayTime).."초", Color, 30)
end

-- 사망 UI 출력
function RenderDeathUI()
    local Rect = RectTransform()
    local Color = Vector4(1,0,0,1)

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
    DrawUIText(Rect, "낙사", Color, 100)
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
    DrawUIText(Rect, "클리어!\n"..string.format("%.1f", PlayTime).."초", Color, 80)
end
