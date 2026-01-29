/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    StarshatterWars
    FILE:         View.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Combined Window + View (legacy UI layer) as plain C++.
    - NOT a UObject (no UCLASS/GENERATED_BODY)
    - Owns rect, visibility, font
    - Supports child views
    - Provides legacy clip + drawing + text API (implemented in View.cpp)

    NOTES
    =====
    - View now carries a Window* for render ownership / draw context.
    - Root views resolve Window from Screen (or are explicitly given a Window).
    - Child views inherit Window from parent automatically.
*/

#pragma once

// ---------------------------------------------------------------------
// Macro collision defense (prevents "View became a macro" disasters).
// ---------------------------------------------------------------------
#ifdef View
#undef View
#endif

#ifdef Screen
#undef Screen
#endif

#ifdef Rect
#undef Rect
#endif

#ifdef List
#undef List
#endif

// ---------------------------------------------------------------------
// Minimal legacy includes (keep header light).
// ---------------------------------------------------------------------
#include "Types.h"
#include "List.h"
#include "Geometry.h"     // Rect

// Minimal UE types used in signatures:
#include "Math/Vector.h"  // FVector
#include "Math/Color.h"   // FColor

// ---------------------------------------------------------------------
// Forward declarations (keep compile dependencies low).
// ---------------------------------------------------------------------
class Screen;
class Window;     // <-- NEW: render ownership context
class Bitmap;
class SystemFont;

// Opaque child list holder to avoid template parse explosions in headers:
struct FViewChildren;

// ---------------------------------------------------------------------
// View
// ---------------------------------------------------------------------
class View
{
public:
    static const char* TYPENAME() { return "View"; }

    // Root view:
    View(Screen* InScreen, int ax, int ay, int aw, int ah);

    // OPTIONAL root view ctor if you want to construct without Screen:
    // (useful when Screen isn't ready yet, or for pure UI previews)
    View(Window* InWindow, int ax, int ay, int aw, int ah);

    // Child view (auto-attaches to parent):
    View(View* InParent, int ax, int ay, int aw, int ah);

    virtual ~View();

    // Identity compare (legacy idiom)
    int operator==(const View& that) const { return this == &that; }

    // ------------------------------------------------------------
    // Context (Screen/Window)
    // ------------------------------------------------------------
    Screen* GetScreen() const { return screen; }
    Window* GetWindow() const { return window; }
    View* GetParent() const { return parent; }

    virtual void SetRectPx(const Rect& R) { ViewRectPx = R; }
    const Rect& GetRectPx() const { return ViewRectPx; }

    // Allows late binding if needed (e.g., before Screen is ready):
    void SetWindow(Window* InWindow) { window = InWindow; }

    // ------------------------------------------------------------
    // Geometry / state
    // ------------------------------------------------------------
    const Rect& GetRect() const { return rect; }
    int   X() const { return rect.x; }
    int   Y() const { return rect.y; }
    int   Width() const { return rect.w; }
    int   Height() const { return rect.h; }

    virtual void Show();
    virtual void Hide();
    virtual bool IsShown() const { return shown; }

    // Move/resize this view (notifies OnWindowMove)
    virtual void MoveTo(const Rect& r);

    // ------------------------------------------------------------
    // Legacy hooks (override in derived views: FadeView/HUDView/etc.)
    // ------------------------------------------------------------
    virtual void Refresh() {}        // draw/update per frame
    virtual void OnWindowMove() {}   // rect changed
    virtual void OnShow() {}         // became visible
    virtual void OnHide() {}         // became hidden

    // ------------------------------------------------------------
    // Child management
    // ------------------------------------------------------------
    virtual bool AddView(View* v);
    virtual bool DelView(View* v);

    // Paint: Refresh self then paint children
    virtual void Paint();

    // ------------------------------------------------------------
    // Clipping (ported from legacy Window)
    // ------------------------------------------------------------
    Rect ClipRect(const Rect& r);
    bool ClipLine(int& x1, int& y1, int& x2, int& y2);
    bool ClipLine(double& x1, double& y1, double& x2, double& y2);

    // ------------------------------------------------------------
    // Drawing (implemented in View.cpp)
    // ------------------------------------------------------------
    void DrawLine(int x1, int y1, int x2, int y2, const FColor& color, int blend = 0);
    void DrawRect(int x1, int y1, int x2, int y2, const FColor& color, int blend = 0);
    void DrawRect(const Rect& r, const FColor& color, int blend = 0);
    void FillRect(int x1, int y1, int x2, int y2, const FColor& color, int blend = 0);
    void FillRect(const Rect& r, const FColor& color, int alpha = 0);

    void DrawBitmap(int x1, int y1, int x2, int y2, Bitmap* img, int blend = 0);
    void FadeBitmap(int x1, int y1, int x2, int y2, Bitmap* img, const FColor& c, int blend);
    void ClipBitmap(int x1, int y1, int x2, int y2, Bitmap* img, const FColor& c, int blend, const Rect& clip);
    void TileBitmap(int x1, int y1, int x2, int y2, Bitmap* img, int blend = 0);

    void DrawLines(int nPts, const FVector* pts, const FColor& color, int blend = 0);
    void DrawPoly(int nPts, const FVector* pts, const FColor& color, int blend = 0);
    void FillPoly(int nPts, const FVector* pts, const FColor& color, int blend = 0);

    void DrawEllipse(int x1, int y1, int x2, int y2, const FColor& color, int blend = 0);
    void FillEllipse(int x1, int y1, int x2, int y2, const FColor& color, int blend = 0);

    // ------------------------------------------------------------
    // Text (implemented in View.cpp)
    // ------------------------------------------------------------
    void        SetFont(SystemFont* f) { font = f; }
    SystemFont* GetFont() const { return font; }

    void                SetStatusColor(SYSTEM_STATUS status);
    void                SetTextColor(FColor TColor);
    void                SetHUDColor(FColor HColor);

    void Print(int x1, int y1, const char* fmt, ...);
    void DrawText(const char* txt, int count, Rect& txt_rect, DWORD flags);

protected:
    // Legacy coordinate transform hooks (if needed later)
    virtual void ScreenToWindow(int& x, int& y) {}
    virtual void ScreenToWindow(Rect& r) {}

protected:
    Rect            rect;
    Rect            ViewRectPx;

    // Logical ownership:
    Screen* screen = nullptr;

    // Render ownership context:
    Window* window = nullptr;  // <-- NEW

    // Hierarchy:
    View* parent = nullptr;
    bool            shown = true;

    // State:
    SystemFont* font = nullptr;

    // Opaque child list (defined in View.cpp) to avoid template issues in headers:
    FViewChildren* children = nullptr;

    // Legacy list (if still used elsewhere):
    List<View>      view_list;

    FColor       HudColor = FColor::Black; 
    FColor       TextColor = FColor::White;
    FColor       StatusColor;
};
