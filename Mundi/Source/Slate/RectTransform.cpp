#include "pch.h"
#include "RectTransform.h"
#include "StatsOverlayD2D.h"

void FDrawInfoText::DrawUI() const
{
    UStatsOverlayD2D& Ins = UStatsOverlayD2D::Get();
    Ins.DrawOnlyText(WText.c_str(), RectTransform.GetRect(Ins.GetViewportSize(), Ins.GetViewportLTop()), Color);
}