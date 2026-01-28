/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe (ported to Unreal)
    FILE:         MFDView.cpp
    AUTHOR:       Carlos Bott
*/

#include "MFDView.h"

#include "SystemFont.h"
#include "FontManager.h"
#include "HUDView.h"
#include "Ship.h"
#include "Sensor.h"
#include "Mouse.h"
#include "Game.h"

#include "Engine/Engine.h"
#include "Framework/Application/SlateApplication.h"
#include "Fonts/SlateFontInfo.h"
#include "Styling/CoreStyle.h"
#include "Rendering/DrawElements.h"

#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogMFDView, Log, All);

// ---- Align flags compatibility (Starshatter uses DT_* style flags) ----
// Keep using your existing DT_LEFT/DT_RIGHT/DT_CENTER/DT_VCENTER/DT_BOTTOM/DT_SINGLELINE definitions
// from Types.h / Win32 compat layer.

UMFDView::UMFDView(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bCanEverTick = true;

    // Initialize default HUD font (legacy name "HUD"):
    HudFont = FontManager::Find("HUD");

    for (int i = 0; i < TXT_LAST; ++i)
    {
        Slots[i].Font = HudFont;
        Slots[i].Color = FColor::White;
        Slots[i].bHidden = true;
    }
}

void UMFDView::SetMFDRect(const FIntRect& InRect)
{
    MFDRect = InRect;
}

void UMFDView::SetHUDColor(const FColor& InHUD, const FColor& InText)
{
    HUDColor = InHUD;
    TextColor = InText;

    // Keep slots coherent:
    for (int i = 0; i < TXT_LAST; ++i)
        Slots[i].Color = TextColor;
}

void UMFDView::SetMode(EMFDMode InMode)
{
    Mode = InMode;

    for (int i = 0; i < TXT_LAST; ++i)
        Slots[i].bHidden = true;

    // Reset animation state counters etc (legacy behavior)
    MouseLatch = 0;
}

void UMFDView::Refresh()
{
    // UView contract: request repaint/update; in UMG this is basically invalidation.
    InvalidateLayoutAndVolatility();
}

void UMFDView::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // We drive latch reset off your legacy Mouse API:
    if (Mouse::LButton() == 0)
        MouseLatch = 0;

    Refresh();
}

int32 UMFDView::NativePaint(
    const FPaintArgs& Args,
    const FGeometry& AllottedGeometry,
    const FSlateRect& MyCullingRect,
    FSlateWindowElementList& OutDrawElements,
    int32 LayerId,
    const FWidgetStyle& InWidgetStyle,
    bool bParentEnabled
) const
{
    PaintElements = &OutDrawElements;
    PaintGeo = &AllottedGeometry;
    PaintLayer = LayerId;

    Draw();

    // Clear paint pointers:
    PaintElements = nullptr;
    PaintGeo = nullptr;

    return PaintLayer;
}

// ------------------ Slate helpers ------------------

FLinearColor UMFDView::Dim(const FLinearColor& C, float Factor)
{
    return FLinearColor(
        C.R * Factor,
        C.G * Factor,
        C.B * Factor,
        C.A
    );
}

FSlateFontInfo UMFDView::GetSlateFont() const
{
    if (HudFont)
        return HudFont->MakeSlateFontInfo();

    return FCoreStyle::GetDefaultFontStyle("Regular", 12);
}

FVector2D UMFDView::MeasureText(const FString& S, const FSlateFontInfo& FontInfo) const
{
    if (FSlateApplication::IsInitialized() && FSlateApplication::Get().GetRenderer())
    {
        TSharedRef<FSlateFontMeasure> Measure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
        return Measure->Measure(S, FontInfo);
    }

    // Safe fallback:
    return FVector2D((float)S.Len() * (float)FontInfo.Size * 0.55f, (float)FontInfo.Size + 4.0f);
}

void UMFDView::DrawLine(const FVector2D& A, const FVector2D& B, const FLinearColor& C, float Thickness) const
{
    if (!PaintElements || !PaintGeo) return;

    TArray<FVector2D> Points;
    Points.Add(A);
    Points.Add(B);

    FSlateDrawElement::MakeLines(
        *PaintElements,
        PaintLayer++,
        PaintGeo->ToPaintGeometry(),
        Points,
        ESlateDrawEffect::None,
        C,
        true,
        Thickness
    );
}

void UMFDView::FillRect(const FSlateRect& R, const FLinearColor& C) const
{
    if (!PaintElements || !PaintGeo) return;

    const FVector2D Pos(R.Left, R.Top);
    const FVector2D Size(R.Right - R.Left, R.Bottom - R.Top);

    FSlateDrawElement::MakeBox(
        *PaintElements,
        PaintLayer++,
        PaintGeo->ToPaintGeometry(Pos, Size),
        FCoreStyle::Get().GetBrush("WhiteBrush"),
        ESlateDrawEffect::None,
        C
    );
}

void UMFDView::DrawRectOutline(const FSlateRect& R, const FLinearColor& C, float Thickness) const
{
    const FVector2D TL(R.Left, R.Top);
    const FVector2D TR(R.Right, R.Top);
    const FVector2D BR(R.Right, R.Bottom);
    const FVector2D BL(R.Left, R.Bottom);

    DrawLine(TL, TR, C, Thickness);
    DrawLine(TR, BR, C, Thickness);
    DrawLine(BR, BL, C, Thickness);
    DrawLine(BL, TL, C, Thickness);
}

void UMFDView::DrawEllipseOutline(const FVector2D& Center, float Rx, float Ry, const FLinearColor& C, float Thickness, int32 Segments) const
{
    if (Segments < 8) Segments = 8;

    TArray<FVector2D> Pts;
    Pts.Reserve(Segments + 1);

    for (int32 i = 0; i <= Segments; ++i)
    {
        const float T = (2.0f * PI) * ((float)i / (float)Segments);
        Pts.Add(FVector2D(
            Center.X + FMath::Cos(T) * Rx,
            Center.Y + FMath::Sin(T) * Ry
        ));
    }

    // Draw polyline:
    for (int32 i = 0; i < Pts.Num() - 1; ++i)
        DrawLine(Pts[i], Pts[i + 1], C, Thickness);
}

void UMFDView::DrawTextAt(const FVector2D& Pos, const FString& Text, const FSlateFontInfo& FontInfo, const FLinearColor& C) const
{
    if (!PaintElements || !PaintGeo) return;

    FSlateDrawElement::MakeText(
        *PaintElements,
        PaintLayer++,
        PaintGeo->ToPaintGeometry(Pos, FVector2D(1, 1)),
        Text,
        FontInfo,
        ESlateDrawEffect::None,
        C
    );
}

// ------------------ MFD main draw ------------------

void UMFDView::Draw() const
{
    if (!PaintElements || !PaintGeo) return;
    if (bHidden || Mode == EMFDMode::OFF) return;

    // Determine widget-local scan rect:
    const float ScanX = (float)MFDRect.Min.X;
    const float ScanY = (float)MFDRect.Min.Y;
    const float ScanW = (float)(MFDRect.Max.X - MFDRect.Min.X);
    const float ScanH = (float)(MFDRect.Max.Y - MFDRect.Min.Y);

    // Mouse-in (legacy Mouse API assumed screen-space; you likely already map it)
    // Here we assume Mouse::X/Y are already in same UI space as your widgets.
    bMouseIn = (Mouse::X() >= (int)ScanX && Mouse::X() <= (int)(ScanX + ScanW) &&
        Mouse::Y() >= (int)ScanY && Mouse::Y() <= (int)(ScanY + ScanH));

    // Hide all slots each frame like the legacy code:
    for (int i = 0; i < TXT_LAST; ++i)
        Slots[i].bHidden = true;

    switch (Mode)
    {
    case EMFDMode::GAME:    DrawGameMFD();   break;
    case EMFDMode::SHIP:    DrawStatusMFD(); break;
    case EMFDMode::FOV:     DrawSensorMFD(); break;
    case EMFDMode::HSD:     DrawHSD();       break;
    case EMFDMode::RADAR3D: Draw3D();        break;
    default: break;
    }
}

// ------------------ Text slots ------------------

void UMFDView::DrawMFDText(int32 DSlot, const FString& Txt, const FSlateRect& R, int32 AlignFlags, int32 Status) const
{
    if (DSlot < 0 || DSlot >= TXT_LAST) return;

    FColor DrawC = TextColor;

    // Status coloring:
    switch (Status)
    {
    default: /* NOMINAL */ break;
    case 1:  DrawC = FColor(255, 255, 0, 255); break; // DEGRADED
    case 2:  DrawC = FColor(255, 0, 0, 255); break;   // CRITICAL
    case 3:  DrawC = FColor(0, 0, 0, 255); break;     // DESTROYED
    }

    // Hover highlight (legacy behavior):
    if (bMouseIn &&
        Mouse::X() >= (int)R.Left && Mouse::X() <= (int)R.Right &&
        Mouse::Y() >= (int)R.Top && Mouse::Y() <= (int)R.Bottom)
    {
        DrawC = FColor::White;
    }

    // All-caps like your existing MFD pass:
    FString Upper = Txt.ToUpper();

    const FSlateFontInfo FontInfo = GetSlateFont();
    const FVector2D Meas = MeasureText(Upper, FontInfo);

    float X = R.Left;
    float Y = R.Top;

    const float RW = R.Right - R.Left;
    const float RH = R.Bottom - R.Top;

    if (AlignFlags & DT_RIGHT)
        X = R.Left + FMath::Max(0.0f, RW - Meas.X);
    else if (AlignFlags & DT_CENTER)
        X = R.Left + FMath::Max(0.0f, (RW - Meas.X) * 0.5f);

    if (AlignFlags & DT_BOTTOM)
        Y = R.Top + FMath::Max(0.0f, RH - Meas.Y);
    else if (AlignFlags & DT_VCENTER)
        Y = R.Top + FMath::Max(0.0f, (RH - Meas.Y) * 0.5f);

    DrawTextAt(FVector2D(X, Y), Upper, FontInfo, FLinearColor(DrawC));

    Slots[DSlot].Rect = R;
    Slots[DSlot].Color = DrawC;
    Slots[DSlot].bHidden = false;
}

void UMFDView::HideMFDText(int32 Slot) const
{
    if (Slot < 0 || Slot >= TXT_LAST) return;
    Slots[Slot].bHidden = true;
}

// ------------------ Minimal ports of the draw passes ------------------
// NOTE: These are faithful to the on-screen behavior (labels, gauge, frames).
// The deep contact rendering (bearing math, contact lists) should be lifted
// from your existing MFD.cpp in the same pattern (lines/rects/ellipse/text calls).

void UMFDView::DrawSensorLabels(const FString& ModeLabel) const
{
    const float ScanX = (float)MFDRect.Min.X;
    const float ScanY = (float)MFDRect.Min.Y;
    const float ScanR = (float)(MFDRect.Max.X - MFDRect.Min.X);

    // Corners:
    DrawMFDText(0, ModeLabel, FSlateRect(ScanX + 2, ScanY + 2, ScanX + 42, ScanY + 14), DT_LEFT);
    DrawMFDText(2, ModeLabel, FSlateRect(ScanX + ScanR - 42, ScanY + 2, ScanX + ScanR - 2, ScanY + 14), DT_RIGHT);
}

void UMFDView::DrawGauge(int32 X, int32 Y, int32 Percent) const
{
    const float BaseX = (float)MFDRect.Min.X + (float)X;
    const float BaseY = (float)MFDRect.Min.Y + (float)Y;

    FillRect(FSlateRect(BaseX, BaseY, BaseX + 53, BaseY + 8), FLinearColor(FColor(64, 64, 64, 255)));

    if (Percent < 3) return;
    Percent = FMath::Clamp(Percent, 0, 100);

    int32 P = Percent / 2;
    FillRect(FSlateRect(BaseX + 2, BaseY + 2, BaseX + 2 + (float)P, BaseY + 7), FLinearColor(FColor(128, 128, 128, 255)));
}

void UMFDView::DrawGameMFD() const
{
    const float ScanX = (float)MFDRect.Min.X;
    const float ScanY = (float)MFDRect.Min.Y;
    const float ScanW = (float)(MFDRect.Max.X - MFDRect.Min.X);

    float Y = ScanY;
    const float LineH = 10.0f;

    if (!HUDView::IsArcade() && HUDView::ShowFPS())
    {
        const FString FPS = FString::Printf(TEXT("FPS: %6.2f"), (double)Game::FrameRate());
        DrawMFDText(0, FPS, FSlateRect(ScanX, Y, ScanX + ScanW, Y + 12), DT_LEFT);
        Y += LineH;
    }

    if (ShipPtr)
    {
        DrawMFDText(1, UTF8_TO_TCHAR(ShipPtr->Name()), FSlateRect(ScanX, Y, ScanX + ScanW, Y + 12), DT_LEFT);
        Y += LineH;
    }

    // Time:
    int32 Hours = (int32)(Game::GameTime() / 3600000);
    int32 Minutes = (int32)(Game::GameTime() / 60000) % 60;
    int32 Seconds = (int32)(Game::GameTime() / 1000) % 60;

    const FString T = (Game::TimeCompression() > 1)
        ? FString::Printf(TEXT("%02d:%02d:%02d x%d"), Hours, Minutes, Seconds, Game::TimeCompression())
        : FString::Printf(TEXT("%02d:%02d:%02d"), Hours, Minutes, Seconds);

    DrawMFDText(2, T, FSlateRect(ScanX, Y, ScanX + ScanW, Y + 12), DT_LEFT);
}

void UMFDView::DrawStatusMFD() const
{
    // Placeholder for your status panel content (systems, shields, power, etc.)
    // Port line-by-line from MFD::DrawStatusMFD using DrawMFDText/DrawGauge/FillRect calls.
    const float ScanX = (float)MFDRect.Min.X;
    const float ScanY = (float)MFDRect.Min.Y;
    const float ScanW = (float)(MFDRect.Max.X - MFDRect.Min.X);

    DrawMFDText(0, TEXT("STATUS"), FSlateRect(ScanX, ScanY, ScanX + ScanW, ScanY + 12), DT_CENTER);
}

void UMFDView::DrawSensorMFD() const
{
    const float ScanX = (float)MFDRect.Min.X;
    const float ScanY = (float)MFDRect.Min.Y;
    const float ScanR = (float)(MFDRect.Max.X - MFDRect.Min.X);

    // Outer ring:
    const FVector2D C(ScanX + ScanR * 0.5f, ScanY + ScanR * 0.5f);
    DrawEllipseOutline(C, ScanR * 0.5f - 4.0f, ScanR * 0.5f - 4.0f, FLinearColor(HUDColor), 1.0f, 48);

    DrawSensorLabels(TEXT("FOV"));
}

void UMFDView::DrawHSD() const
{
    const float ScanX = (float)MFDRect.Min.X;
    const float ScanY = (float)MFDRect.Min.Y;
    const float ScanR = (float)(MFDRect.Max.X - MFDRect.Min.X);

    const FVector2D C(ScanX + ScanR * 0.5f, ScanY + ScanR * 0.5f);

    // HSD ring:
    DrawEllipseOutline(C, ScanR * 0.5f - 4.0f, ScanR * 0.5f - 4.0f, FLinearColor(HUDColor), 1.0f, 48);

    DrawSensorLabels(TEXT("HSD"));
}

void UMFDView::Draw3D() const
{
    const float ScanX = (float)MFDRect.Min.X;
    const float ScanY = (float)MFDRect.Min.Y;
    const float ScanR = (float)(MFDRect.Max.X - MFDRect.Min.X);

    const FVector2D C(ScanX + ScanR * 0.5f, ScanY + ScanR * 0.5f);

    // 3D radar frame:
    DrawEllipseOutline(C, ScanR * 0.5f - 4.0f, (ScanR * 0.5f - 4.0f) * 0.6f, FLinearColor(HUDColor), 1.0f, 48);

    DrawSensorLabels(TEXT("3D"));
}
