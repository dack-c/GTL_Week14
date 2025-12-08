#pragma once
#include "Vector.h"
#include "d2d1.h"
struct FRectTransform
{
public:
    FVector2D AnchorMax = FVector2D(0, 0);
    FVector2D AnchorMin = FVector2D(0, 0);
    FVector2D Anchor = FVector2D(0, 0);
    FVector2D Pivot = FVector2D(0.5f, 0.5f);
    FVector2D Pos;
    FVector2D Size;
    int ZOrder = 0;
    FRectTransform(const FVector2D& InPos, const FVector2D& InSize) : Pos(InPos), Size(InSize){}
    D2D1_RECT_F GetRect(const FVector2D& ViewportSize, const FVector2D& ViewportLTop) const
    {
        D2D1_RECT_F Rect;
        FVector2D AnchorPos = FVector2D(ViewportSize.X * Anchor.X, ViewportSize.Y * Anchor.Y) + ViewportLTop;
        FVector2D PivotPos = AnchorPos + Pos;
        Rect.left = PivotPos.X - Pivot.X * Size.X;
        Rect.right = PivotPos.X + (1 - Pivot.X) * Size.X;
        Rect.bottom = PivotPos.Y - Pivot.Y * Size.Y;
        Rect.top = PivotPos.Y + (1 - Pivot.Y) * Size.Y;
        return Rect;
    }
};

struct FDrawInfo
{
    FRectTransform RectTransform;
    FDrawInfo(const FRectTransform& InRT) : RectTransform(InRT) {}
    virtual void DrawUI() const = 0;
    virtual ~FDrawInfo() = default;
    bool operator<(const FDrawInfo& Other) const
    {
        return RectTransform.ZOrder < Other.RectTransform.ZOrder;
    }
};
struct FDrawInfoText : public FDrawInfo
{
    FVector4 Color;
    FWideString WText;
    float FontSize;
    ~FDrawInfoText() override = default;

    FDrawInfoText(const FRectTransform& InRT, const FString& InText, const FVector4& InColor, const float InFontSize) :
        FDrawInfo(InRT), WText(UTF8ToWide(InText)), Color(InColor), FontSize(InFontSize)
    {
    
    }

    FDrawInfoText(const FRectTransform& InRT, const FWideString& InWText, const FVector4& InColor, const float InFontSize) :
        FDrawInfo(InRT), WText(InWText), Color(InColor), FontSize(InFontSize)
    {

    }
    void DrawUI() const override;
};
struct FDrawInfoSprite : public FDrawInfo
{
    FWideString SpritePath;
    float Opacity = 1.0f;
    ~FDrawInfoSprite() override = default;
    FDrawInfoSprite(const FRectTransform& InRT, const FWideString& InFilePath, const float InOpacity = 1.0f) : 
        FDrawInfo(InRT), SpritePath(InFilePath), Opacity (InOpacity){}
    FDrawInfoSprite(const FRectTransform& InRT, const FString& InFilePath, const float InOpacity = 1.0f) :
        FDrawInfo(InRT), SpritePath(UTF8ToWide(InFilePath)), Opacity(InOpacity) {}
    void DrawUI() const override;
};