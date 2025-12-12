#pragma once
#include <d2d1_1.h>
#include <dwrite.h>
#include <dwrite_3.h>
#include <wincodec.h>
#include "RectTransform.h"

struct IDWriteFontCollection1;

class UStatsOverlayD2D
{
public:
    static UStatsOverlayD2D& Get();

    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context, IDXGISwapChain* swapChain);
	void Shutdown();
    void Draw();

    void LoadFontsFromDirectory(const FString& DirectoryPath);

    void ReadBitmap(const FWideString& FilePath);
    void ReadBitmap(const FString& FilePath);
    void DrawOnlyText(const wchar_t* InText, const D2D1_RECT_F& InRect, const FVector4& Color, const float FontSize, const wchar_t* FontName = L"Segoe UI");
    void DrawRect(const D2D1_RECT_F& InRect, const FVector4& Color, float StrokeWidth = 2.0f);
    void DrawBitmap(const D2D1_RECT_F& InRect, const FString& FilePath, const float Opacity = 1.0f) const;
    void DrawBitmap(const D2D1_RECT_F& InRect, const FWideString& FilePath, const float Opacity = 1.0f) const;

    void SetShowFPS(bool b) { bShowFPS = b; }
    void SetShowMemory(bool b) { bShowMemory = b; }
    void SetShowPicking(bool b)  { bShowPicking = b; }
    void SetShowDecal(bool b)  { bShowDecal = b; }
    void SetShowTileCulling(bool b)  { bShowTileCulling = b; }
    void SetShowLights(bool b) { bShowLights = b; }
    void SetShowShadow(bool b) { bShowShadow = b; }
    void SetShowSkinning(bool b) { bShowSkinning = b; }
    void SetShowParticle(bool b) { bShowParticle = b; }
    void ToggleFPS() { bShowFPS = !bShowFPS; }
    void ToggleMemory() { bShowMemory = !bShowMemory; }
    void TogglePicking() { bShowPicking = !bShowPicking; }
    void ToggleDecal() { bShowDecal = !bShowDecal; }
    void ToggleTileCulling() { bShowTileCulling = !bShowTileCulling; }
    void ToggleLights() { bShowLights = !bShowLights; }
    void ToggleShadow() { bShowShadow = !bShowShadow; }
    void ToggleSkinning() { bShowSkinning = !bShowSkinning; }
    void ToggleParticle() { bShowParticle = !bShowParticle; }
    bool IsFPSVisible() const { return bShowFPS; }
    bool IsMemoryVisible() const { return bShowMemory; }
    bool IsPickingVisible() const { return bShowPicking; }
    bool IsDecalVisible() const { return bShowDecal; }
    bool IsTileCullingVisible() const { return bShowTileCulling; }
    bool IsLightsVisible() const { return bShowLights; }
    bool IsShadowVisible() const { return bShowShadow; }
    bool IsSkinningVisible() const { return bShowSkinning; }
    bool IsParticleVisible() const { return bShowParticle; }

    void RegisterTextUI(const FRectTransform& InRectTransform, const FString& Text, const FVector4& Color, const float InFontSize, const FString& InFontName = "Segoe UI");
    void RegisterRectUI(const FRectTransform& InRectTransform, const FVector4& Color, float StrokeWidth = 2.0f);
    void RegisterSpriteUI(const FRectTransform& InRectTransform, const FString& FilePath, const float Opacity = 1.0f);

    FVector2D GetViewportSize() const
    {
        return ViewportSize;
    }
    FVector2D GetViewportLTop() const
    {
        return ViewportLTop;
    }
private:
    UStatsOverlayD2D() = default;
    ~UStatsOverlayD2D() = default;
    UStatsOverlayD2D(const UStatsOverlayD2D&) = delete;
    UStatsOverlayD2D& operator=(const UStatsOverlayD2D&) = delete;

    void EnsureInitialized();
    void ReleaseD2DResources();

    TArray<FDrawInfo*> DrawInfose;
    TMap<FWideString, ID2D1Bitmap*> BitmapMap;


private:

    FVector2D ViewportLTop;
    FVector2D ViewportSize;

    bool bInitialized = false;
    bool bShowFPS = false;
    bool bShowMemory = false;
    bool bShowPicking = false;
    bool bShowDecal = false;
    bool bShowTileCulling = false;
    bool bShowShadow = false;
    bool bShowLights = false;
    bool bShowSkinning = false;
    bool bShowParticle = false;

    ID3D11Device* D3DDevice = nullptr;
    ID3D11DeviceContext* D3DContext = nullptr;
    IDXGISwapChain* SwapChain = nullptr;
    
    ID2D1Factory1* D2DFactory = nullptr;
    ID2D1Device* D2DDevice = nullptr;
    ID2D1DeviceContext* D2DContext = nullptr;
    IDWriteFactory* DWriteFactory = nullptr;
    IDWriteTextFormat* TextFormat = nullptr;
    IWICImagingFactory* WICFactory = nullptr;

    IDWriteFontCollection1* CustomFontCollection = nullptr;

    ID2D1SolidColorBrush* BrushYellow = nullptr;
    ID2D1SolidColorBrush* BrushSkyBlue = nullptr;
    ID2D1SolidColorBrush* BrushLightGreen = nullptr;
    ID2D1SolidColorBrush* BrushOrange = nullptr;
    ID2D1SolidColorBrush* BrushCyan = nullptr;
    ID2D1SolidColorBrush* BrushViolet = nullptr;
    ID2D1SolidColorBrush* BrushDeepPink = nullptr;
    ID2D1SolidColorBrush* BrushBlack = nullptr;

    ID2D1SolidColorBrush* UIColorBrush = nullptr;
    IDWriteTextFormat* UITextFormat = nullptr;
};
