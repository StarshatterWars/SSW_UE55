/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (C) 2025-2026.

    SUBSYSTEM:    Stars.exe (ported to Unreal)
    FILE:         MFDView.h
    AUTHOR:       Carlos Bott
    ORIGINAL:     John DiCamillo / Destroyer Studios LLC (1997-2004)

    OVERVIEW
    ========
    MFDView
    - View-derived Multi Function Display renderer (legacy MFD port)
    - UE-friendly types (FColor, enum class)
    - Does not require HUDText from HUDView; maintains its own text slots
*/

#pragma once

#include "CoreMinimal.h"

#include "Types.h"     // Rect, Text (legacy)
#include "View.h"      // base class (Window + draw pipeline)
#include "SystemFont.h"

class Window;
class Ship;
class Sprite;
class CameraView;
class Bitmap;

class MFDView : public View
{
public:
    static const char* TYPENAME() { return "MFDView"; }

    MFDView(Window* InWindow, int32 InIndex);
    virtual ~MFDView();

    // Static color shared across MFDs (legacy behavior)
    static void   SetColor(const FColor& InColor);

    // Operations:
    virtual void  Draw();
    virtual void  DrawGameMFD();
    virtual void  DrawStatusMFD();
    virtual void  DrawSensorMFD();
    virtual void  DrawHSD();
    virtual void  Draw3D();
    virtual void  DrawSensorLabels(const char* MfdModeLabel);
    virtual void  DrawMap();
    virtual void  DrawGauge(int32 X, int32 Y, int32 Percent);
    virtual void  SetStatusColor(int32 Status);

    // Plumbing:
    virtual void         SetRect(const Rect& InRect);
    virtual const Rect& GetRect() const { return RectPx; }

    virtual void         SetMode(EMFDMode InMode);
    virtual EMFDMode     GetMode() const { return Mode; }

    virtual void         SetShip(Ship* InShip) { ShipPtr = InShip; }
    virtual Ship* GetShip() const { return ShipPtr; }

    virtual void         Show();
    virtual void         Hide();
    virtual bool         IsHidden() const { return bHidden; }

    virtual void         UseCameraView(CameraView* InView) { CamView = InView; }

    void                 SetCockpitHUDTexture(Bitmap* InBmp) { CockpitHUDTexture = InBmp; }
    void                 SetText3DColor(const FColor& InColor);

    bool                 IsMouseLatched() const;

    // Text helpers (legacy-like)
    void                 DrawMFDText(int32 Slot, const char* Txt, Rect& R, uint32 AlignFlags, int32 Status = -1);
    void                 HideMFDText(int32 Slot);

private:
    struct FMFDText
    {
        SystemFont* Font = nullptr;
        Rect        R;
        FColor      Color = FColor::White;
        bool        bHidden = true;
    };

private:
    // Convert narrow -> wide for SystemFont::DrawTextW
    static void ToWide(const char* In, wchar_t* Out, int32 OutChars);

    // Draw a simple frame and title
    void DrawFrameAndHeader(const char* Header);

private:
    static FColor  GMFDColor;

    Rect           RectPx;
    int32          Index;
    EMFDMode       Mode;
    int32          Lines;

    Sprite* SpritePtr;
    bool           bHidden;

    Ship* ShipPtr;
    CameraView* CamView;
    Bitmap* CockpitHUDTexture;

    // Mouse latch behavior (matches HUDView logic)
    int32          MouseLatch;
    bool           bMouseIn;

    // Local text slots (legacy had TXT_LAST=20)
    static constexpr int32 TXT_LAST = 20;
    FMFDText       TextSlots[TXT_LAST];

    // Fonts (pulled from your SystemFont/FontManager layer)
    SystemFont* HudFont;
    SystemFont* BigFont; #include "MFDView.h"

#include "Bitmap.h"
#include "Window.h"
#include "Mouse.h"
#include "FontManager.h"   // whatever you use to Find("HUD") / Find("GUI")

        // Forward-only (keep compile dependencies light):
#include "Ship.h"
#include "CameraView.h"

        FColor MFDView::GMFDColor = FColor(0, 255, 255); // default cyan-ish like legacy

    MFDView::MFDView(Window* InWindow, int32 InIndex)
        : View(InWindow, 0, 0, 0, 0)
        , RectPx(0, 0, 0, 0)
        , Index(InIndex)
        , Mode(EMFDMode::OFF)
        , Lines(0)
        , SpritePtr(nullptr)
        , bHidden(false)
        , ShipPtr(nullptr)
        , CamView(nullptr)
        , CockpitHUDTexture(nullptr)
        , MouseLatch(0)
        , bMouseIn(false)
        , HudFont(nullptr)
        , BigFont(nullptr)
        , TextColor(FColor::White)
        , StatusColor(FColor::White)
    {
        // Fonts (match HUDView pattern)
        HudFont = FontManager::Find("HUD");
        BigFont = FontManager::Find("GUI");

        for (int32 i = 0; i < TXT_LAST; ++i)
        {
            TextSlots[i].Font = HudFont;
            TextSlots[i].Color = TextColor;
            TextSlots[i].bHidden = true;
        }
    }

    MFDView::~MFDView()
    {
    }

    void MFDView::SetColor(const FColor& InColor)
    {
        GMFDColor = InColor;
    }

    void MFDView::SetRect(const Rect& InRect)
    {
        RectPx = InRect;

        // Also update View’s rect so it can use ax/ay/aw/ah semantics if needed:
        ax = RectPx.x;
        ay = RectPx.y;
        aw = RectPx.w;
        ah = RectPx.h;
    }

    void MFDView::SetMode(EMFDMode InMode)
    {
        Mode = InMode;

        // Reset any text slots when switching:
        for (int32 i = 0; i < TXT_LAST; ++i)
            TextSlots[i].bHidden = true;
    }

    void MFDView::Show()
    {
        bHidden = false;
    }

    void MFDView::Hide()
    {
        bHidden = true;
        for (int32 i = 0; i < TXT_LAST; ++i)
            TextSlots[i].bHidden = true;
    }

    bool MFDView::IsMouseLatched() const
    {
        return (MouseLatch != 0) || bMouseIn;
    }

    void MFDView::SetText3DColor(const FColor& InColor)
    {
        TextColor = InColor;
        for (int32 i = 0; i < TXT_LAST; ++i)
            TextSlots[i].Color = InColor;
    }

    void MFDView::SetStatusColor(int32 Status)
    {
        // Minimal port: you can map your SYSTEM_STATUS / System::STATUS here.
        // Keep it conservative so you don’t regress HUDView logic.
        switch (Status)
        {
        default:
        case 0:  StatusColor = TextColor;                break; // nominal
        case 1:  StatusColor = FColor(255, 255, 0);      break; // degraded
        case 2:  StatusColor = FColor(255, 64, 64);      break; // critical
        }
    }

    void MFDView::Draw()
    {
        if (bHidden || !window)
            return;

        // Clear latch if mouse up:
        if (Mouse::LButton() == 0)
            MouseLatch = 0;

        bMouseIn = false;

        // Frame + dispatch:
        switch (Mode)
        {
        case EMFDMode::OFF:
            DrawFrameAndHeader("MFD OFF");
            break;

        case EMFDMode::GAME:
            DrawGameMFD();
            break;

        case EMFDMode::SHIP:
            DrawStatusMFD();
            break;

        case EMFDMode::FOV:
            DrawSensorMFD();
            break;

        case EMFDMode::HSD:
            DrawHSD();
            break;

        case EMFDMode::RADAR3D:
            Draw3D();
            break;

        default:
            DrawFrameAndHeader("MFD");
            break;
        }
    }

    void MFDView::DrawFrameAndHeader(const char* Header)
    {
        // Border:
        window->DrawRect(
            RectPx.x,
            RectPx.y,
            RectPx.x + RectPx.w,
            RectPx.y + RectPx.h,
            GMFDColor
        );

        // Title bar:
        Rect titleR(RectPx.x + 4, RectPx.y + 2, RectPx.w - 8, 12);
        DrawMFDText(0, Header, titleR, DT_LEFT | DT_SINGLELINE);
    }

    void MFDView::DrawGameMFD()
    {
        DrawFrameAndHeader("GAME");

        Rect r(RectPx.x + 6, RectPx.y + 18, RectPx.w - 12, 12);
        DrawMFDText(1, "STATUS / NAV / WEAP", r, DT_LEFT | DT_SINGLELINE);

        r.y += 12;
        DrawMFDText(2, "MFDVIEW LEGACY PORT", r, DT_LEFT | DT_SINGLELINE);
    }

    void MFDView::DrawStatusMFD()
    {
        DrawFrameAndHeader("SHIP");

        // Minimal “ship status” panel (you can expand using ShipPtr fields)
        Rect r(RectPx.x + 6, RectPx.y + 18, RectPx.w - 12, 12);

        if (!ShipPtr)
        {
            DrawMFDText(1, "NO SHIP", r, DT_LEFT | DT_SINGLELINE);
            return;
        }

        DrawMFDText(1, "SHIP STATUS", r, DT_LEFT | DT_SINGLELINE);

        // Example gauge block:
        DrawGauge(RectPx.x + 10, RectPx.y + 36, 75);
    }

    void MFDView::DrawSensorMFD()
    {
        DrawFrameAndHeader("FOV");

        Rect r(RectPx.x + 6, RectPx.y + 18, RectPx.w - 12, 12);
        DrawMFDText(1, "SENSOR VIEW", r, DT_LEFT | DT_SINGLELINE);

        DrawSensorLabels("FOV");
    }

    void MFDView::DrawHSD()
    {
        DrawFrameAndHeader("HSD");

        // Simple “radar ring” placeholder:
        const int32 cx = RectPx.x + RectPx.w / 2;
        const int32 cy = RectPx.y + RectPx.h / 2;

        const int32 rad = FMath::Min(RectPx.w, RectPx.h) / 2 - 10;

        window->DrawEllipse(cx - rad, cy - rad, cx + rad, cy + rad, GMFDColor);
        window->DrawLine(cx - rad, cy, cx + rad, cy, GMFDColor);
        window->DrawLine(cx, cy - rad, cx, cy + rad, GMFDColor);
    }

    void MFDView::Draw3D()
    {
        DrawFrameAndHeader("3D");

        Rect r(RectPx.x + 6, RectPx.y + 18, RectPx.w - 12, 12);
        DrawMFDText(1, "RADAR3D", r, DT_LEFT | DT_SINGLELINE);

        // Keep it simple for now; you can expand using CamView / projector later.
        r.y += 12;
        DrawMFDText(2, "PLACEHOLDER GRID", r, DT_LEFT | DT_SINGLELINE);

        // small grid:
        for (int32 i = 0; i < 5; ++i)
        {
            const int32 x0 = RectPx.x + 10;
            const int32 x1 = RectPx.x + RectPx.w - 10;
            const int32 y = RectPx.y + 40 + i * 12;
            window->DrawLine(x0, y, x1, y, FColor(64, 64, 64));
        }
    }

    void MFDView::DrawSensorLabels(const char* MfdModeLabel)
    {
        Rect r(RectPx.x + 6, RectPx.y + RectPx.h - 14, RectPx.w - 12, 12);

        char buf[64] = { 0 };
#if defined(_MSC_VER)
        sprintf_s(buf, "%s", MfdModeLabel ? MfdModeLabel : "MFD");
#else
        snprintf(buf, sizeof(buf), "%s", MfdModeLabel ? MfdModeLabel : "MFD");
#endif

        DrawMFDText(3, buf, r, DT_RIGHT | DT_SINGLELINE);
    }

    void MFDView::DrawMap()
    {
        DrawFrameAndHeader("MAP");
        // (optional later)
    }

    void MFDView::DrawGauge(int32 X, int32 Y, int32 Percent)
    {
        Percent = FMath::Clamp(Percent, 0, 100);

        const int32 w = 80;
        const int32 h = 8;

        // Outline:
        window->DrawRect(X, Y, X + w, Y + h, GMFDColor);

        // Fill:
        const int32 fw = (w - 2) * Percent / 100;
        if (fw > 0)
        {
            window->FillRect(X + 1, Y + 1, X + 1 + fw, Y + h - 1, GMFDColor);
        }

        // Label:
        char buf[32] = { 0 };
#if defined(_MSC_VER)
        sprintf_s(buf, "%d%%", Percent);
#else
        snprintf(buf, sizeof(buf), "%d%%", Percent);
#endif

        Rect tr(X + w + 6, Y - 2, 50, 12);
        DrawMFDText(4, buf, tr, DT_LEFT | DT_SINGLELINE);
    }

    void MFDView::HideMFDText(int32 Slot)
    {
        if (Slot < 0 || Slot >= TXT_LAST)
            return;

        TextSlots[Slot].bHidden = true;
    }

    void MFDView::ToWide(const char* In, wchar_t* Out, int32 OutChars)
    {
        if (!Out || OutChars <= 0)
            return;

        Out[0] = 0;

        if (!In)
            return;

#if defined(_MSC_VER)
        mbstowcs_s(nullptr, Out, OutChars, In, _TRUNCATE);
#else
        mbstowcs(Out, In, (size_t)(OutChars - 1));
        Out[OutChars - 1] = 0;
#endif
    }

    void MFDView::DrawMFDText(int32 Slot, const char* Txt, Rect& R, uint32 AlignFlags, int32 Status)
    {
        if (Slot < 0 || Slot >= TXT_LAST)
            return;

        FMFDText& T = TextSlots[Slot];

        SystemFont* F = T.Font ? T.Font : HudFont;
        if (!F)
            return;

        // Status tint:
        FColor c = T.Color;
        if (Status >= 0)
        {
            SetStatusColor(Status);
            c = StatusColor;
        }

        // Mouse-over latch region (legacy behavior):
        if (!CockpitHUDTexture && R.Contains(Mouse::X(), Mouse::Y()))
        {
            bMouseIn = true;

            if (Mouse::LButton() && MouseLatch == 0)
                MouseLatch = 1;
        }

        F->SetColor(c);

        // Use the SystemFont wide path to avoid the DrawTextW overload mismatch later:
        wchar_t wbuf[256];
        ToWide(Txt ? Txt : "", wbuf, UE_ARRAY_COUNT(wbuf));

        // NOTE: clip for DrawTextW in SystemFont is a Rect, matches your header.
        // Align flags are legacy DT_* flags; your SystemFont::DrawText handles those.
        // If you prefer, swap to F->DrawText(Txt, -1, R, AlignFlags) once your DrawText implementation is stable.
        F->DrawTextW(wbuf, -1, R.x, R.y, R, nullptr);

        T.R = R;
        T.bHidden = false;
    }


    // Per-draw colors:
    FColor         TextColor;
    FColor         StatusColor;
};
