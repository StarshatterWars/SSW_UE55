/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         WepView.h
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO:
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    View class for Tactical HUD Overlay
*/

#pragma once

#include "View.h"
#include "SimObject.h"   // for SimObserver / SimObject
#include "SimSystem.h"      // for Rect (core type in Starshatter)
#include "Color.h"

// Forward declare Unreal texture type (Bitmap replacement):
class UTexture2D;

// +--------------------------------------------------------------------+

class Graphic;
class Sprite;
class Ship;
class SimContact;
class HUDView;
class Sim;
class SimRegion;

// +--------------------------------------------------------------------+

class WepView : public View, public SimObserver
{
public:
    WepView(Window* c);
    virtual ~WepView();

    // Operations:
    virtual void            Refresh();
    virtual void            OnWindowMove();
    virtual void            ExecFrame();
    virtual void            SetOverlayMode(int mode);
    virtual int             GetOverlayMode() const { return mode; }
    virtual void            CycleOverlayMode();

    virtual void            RestoreOverlay();

    virtual bool            Update(SimObject* obj);
    virtual const char*     GetObserverName() const;

    static WepView*         GetInstance() { return wep_view; }
    static void             SetColor(FColor c);

    static bool             IsMouseLatched();

protected:
    void              DrawOverlay();

    void              DoMouseFrame();
    bool              CheckButton(int index, int x, int y);
    void              CycleSubTarget(int direction);

    int               mode = 0;
    int               transition = 0;
    int               mouse_down = 0;
    int               width = 0, height = 0, aw = 0, ah = 0;
    double            xcenter = 0.0, ycenter = 0.0;

    Sim* sim = nullptr;
    Ship* ship = nullptr;
    SimObject* target = nullptr;
    HUDView* hud = nullptr;

    enum { MAX_WEP = 4, MAX_BTN = 16 };
    Rect        btn_rect[MAX_BTN];

    SimRegion* active_region = nullptr;

    static WepView* wep_view;
};


