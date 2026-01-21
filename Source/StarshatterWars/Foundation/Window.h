/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    nGenEx.lib
    FILE:         Window.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Window class (a region of a screen or buffer)
*/

#pragma once

#include "Types.h"
#include "Geometry.h"
#include "List.h"
#include "SystemFont.h"

// Minimal Unreal includes (kept tight):
#include "Math/Vector.h"   // FVector
#include "Math/Color.h"    // FColor

// +--------------------------------------------------------------------+

class Bitmap;
class Font;
class Screen;
class View;

// +--------------------------------------------------------------------+

class Window
{
    friend class Screen;

public:
    static const char* TYPENAME() { return "Window"; }

    Window(Screen* s, int ax, int ay, int aw, int ah);
    virtual ~Window();

    int operator == (const Window& that) const { return this == &that; }

    // Screen dimensions:
    Screen* GetScreen() const { return screen; }
    const Rect& GetRect()   const { return rect; }
    int         X()         const { return rect.x; }
    int         Y()         const { return rect.y; }
    int         Width()     const { return rect.w; }
    int         Height()    const { return rect.h; }

    // UE helper: treat FVector as a 2D point in window/screen space (X,Y); Z ignored.
    static inline int   PtX(const FVector& p) { return (int)p.X; }
    static inline int   PtY(const FVector& p) { return (int)p.Y; }
    static inline void  PtXY(const FVector& p, int& outX, int& outY) { outX = (int)p.X; outY = (int)p.Y; }

    // Operations:
    virtual void Paint();
    virtual void Show() { shown = true; }
    virtual void Hide() { shown = false; }
    virtual bool IsShown()     const { return shown; }

    virtual void MoveTo(const Rect& r);

    virtual bool AddView(View* v);
    virtual bool DelView(View* v);

    Rect ClipRect(const Rect& r);
    bool ClipLine(int& x1, int& y1, int& x2, int& y2);
    bool ClipLine(double& x1, double& y1, double& x2, double& y2);

    void DrawLine(int x1, int y1, int x2, int y2, const FColor& color, int blend = 0);
    void DrawRect(int x1, int y1, int x2, int y2, const FColor& color, int blend = 0);
    void DrawRect(const Rect& r, const FColor& color, int blend = 0);
    void FillRect(int x1, int y1, int x2, int y2, const FColor& color, int blend = 0);
    void FillRect(const Rect& r, const FColor& color, int alpha = 0);
    void DrawBitmap(int x1, int y1, int x2, int y2, Bitmap* img, int blend = 0);
    void FadeBitmap(int x1, int y1, int x2, int y2, Bitmap* img, const FColor& c, int blend);
    void ClipBitmap(int x1, int y1, int x2, int y2, Bitmap* img, const FColor& c, int blend, const Rect& clip);
    void TileBitmap(int x1, int y1, int x2, int y2, Bitmap* img, int blend = 0);

    // UE: POINT -> FVector (use pts[i].X, pts[i].Y; Z ignored)
    void DrawLines(int nPts, const FVector* pts, const FColor& color, int blend = 0);
    void DrawPoly(int nPts, const FVector* pts, const FColor& color, int blend = 0);
    void FillPoly(int nPts, const FVector* pts, const FColor& color, int blend = 0);

    void DrawEllipse(int x1, int y1, int x2, int y2, const FColor& color, int blend = 0);
    void FillEllipse(int x1, int y1, int x2, int y2, const FColor& color, int blend = 0);

    // text methods:
    void  SetFont(SystemFont* f) { font = f; }
    SystemFont* GetFont()       const { return font; }

    void Print(int x1, int y1, const char* fmt, ...);
    void DrawText(const char* txt, int count, Rect& txt_rect, DWORD flags);

protected:
    // translate screen coords into window relative coords
    virtual void ScreenToWindow(int& x, int& y) {}
    virtual void ScreenToWindow(Rect& r) {}

    Rect        rect;
    Screen*     screen;
    bool        shown;
    SystemFont* font;

    List<View>  view_list;
};

// +--------------------------------------------------------------------+
