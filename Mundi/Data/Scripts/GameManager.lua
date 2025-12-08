function BeginPlay()
    InitGame()
    GlobalConfig.GameState = "Init"
    -- GlobalConfig.Camera1 = FindObjectByName("Camera1")
    -- GlobalConfig.Camera2 = FindObjectByName("Camera2")
    -- GlobalConfig.Camera3 = FindObjectByName("Camera3")
    -- GlobalConfig.Camera4 = FindObjectByName("Camera4")
    -- GlobalConfig.Camera5 = FindObjectByName("Camera5")

    -- StartCoroutine(MoveCameras)

    -- GetCameraManager():StartCameraShake(20, 0.1, 0.1,40)
    -- Color(0.0, 0.2, 0.4, 1.0)
    -- GetCameraManager():StartFade(5.0, 0, 1.0, Color(0.0, 0.2, 0.4, 1.0), 0)
    -- GetCameraManager():StartLetterBox(2.0, 1.777, 0, Color(0.0, 0.2, 0.4, 1.0))
    -- GetCameraManager():StartVignette(4.0, 0.2, 0.5, 10.0, 2.0, Color(0.9, 0.0, 0.2, 0.0), 0)
end

function MoveCameras()    
    -- GetCameraManager():SetViewTarget(GetComponent(GlobalConfig.Camera1, "UCameraComponent"))
    -- coroutine.yield("wait_time", 4)
    -- DeleteObject(GlobalConfig.teamName)
    -- GetCameraManager():SetViewTargetWithBlend(GetComponent(GlobalConfig.Camera2, "UCameraComponent"), 4)
    -- coroutine.yield("wait_time", 4)
    -- GetCameraManager():SetViewTargetWithBlend(GetComponent(GlobalConfig.Camera3, "UCameraComponent"), 4)
    -- coroutine.yield("wait_time", 4)
    -- GetCameraManager():SetViewTargetWithBlend(GetComponent(GlobalConfig.Camera4, "UCameraComponent"), 4)
    -- coroutine.yield("wait_time", 2.5)
    -- GetCameraManager():SetViewTargetWithBlend(GetComponent(GlobalConfig.Camera5, "UCameraComponent"), 2.5)
    -- coroutine.yield("wait_time", 4)
    -- GetCameraManager():SetViewTargetWithBlend(GetComponent(GlobalConfig.Camera1, "UCameraComponent"), 4)
    -- coroutine.yield("wait_time", 4)
    -- GlobalConfig.GameState = "Init"
    
    -- GetCameraManager():StartGamma(3.0 /2.2)
end

function EndPlay()
end

function OnBeginOverlap(OtherActor)
end

function OnEndOverlap(OtherActor)
end


-- 게임 상태 초기화
function InitGame()
    -- TODO: 플레이어 생성
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

        if InputManager:IsKeyDown(" ") then
            GlobalConfig.GameState = "Start"
            InitGame()
        end
    
    elseif GlobalConfig.GameState == "Start" then
        -- 시작 연출 이 끝나면 Playing 으로 전환됨
        GlobalConfig.bIsGameClear = false
        GlobalConfig.bIsPlayerDeath = false
        GlobalConfig.GameState = "Playing"

    elseif GlobalConfig.GameState == "Playing" then
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
        
        if InputManager:IsKeyDown("R") then
            GlobalConfig.GameState = "Init"
        end

    elseif GlobalConfig.GameState == "Clear" then
        -- 클리어 타임 표시
        RenderClearUI()
        
        if InputManager:IsKeyDown("R") then
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
    DrawUIText(Rect, "Press [Space Bar] To Start", Color, 60)
end

-- 인게임 UI 출력
function RenderInGameUI()
    local Rect = RectTransform()
    local Color = Vector4(1,1,1,1)

    AnchorMin = Vector2D(0,0)
    AnchorMax = Vector2D(0.3,0.2)
    Rect = FRectTransform.CreateAnchorRange(AnchorMin,AnchorMax)
    Rect.ZOrder = 1;
    DrawUIText(Rect, "남은 거리: 50m", Color, 30)
end

-- 사망 UI 출력
function RenderDeathUI()
    local Rect = RectTransform()
    local Color = Vector4(1,1,1,1)

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
    DrawUIText(Rect, "낙사", Color, 80)
end

-- 클리어 UI 출력
function RenderClearUI()
    local Rect = RectTransform()
    local Color = Vector4(1,1,1,1)

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
    DrawUIText(Rect, "클리어!", Color, 80)
end
