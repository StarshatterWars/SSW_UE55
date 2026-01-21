/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         HUDView.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    View class for Heads Up Display
*/

#pragma once

#include "Types.h"
#include "View.h"
#include "SystemFont.h"
#include "SimSystem.h"
#include "SimObject.h"
#include "Text.h"

// Minimal Unreal includes required for FVector / FColor:
#include "Math/Vector.h"
#include "Math/Color.h"

// +--------------------------------------------------------------------+

class Bitmap;
class Graphic;
class Sprite;
class Solid;
class Ship;
class SimContact;
class Physical;
class OrbitalBody;
class OrbitalRegion;
class Instruction;
class CameraView;
class SimProjector;
class MFD;
class Sim;
class SimRegion;

// +--------------------------------------------------------------------+

class HUDView : public View,
    public SimObserver
{
public:
    HUDView(Window* c);
    virtual ~HUDView();

    enum HUDModes { HUD_MODE_OFF, HUD_MODE_TAC, HUD_MODE_NAV, HUD_MODE_ILS };

    // Operations:
    virtual void       Refresh();
    virtual void       OnWindowMove();
    virtual void       ExecFrame();
    virtual void       UseCameraView(CameraView* v);

    virtual Ship* GetShip()   const { return ship; }
    virtual SimObject* GetTarget() const { return target; }
    virtual void       SetShip(Ship* s);
    virtual void       SetTarget(SimObject* t);
    virtual MFD* GetMFD(int n) const;

    virtual void       HideAll();
    virtual void       DrawBars();
    virtual void       DrawNav();
    virtual void       DrawILS();
    virtual void       DrawObjective();
    virtual void       DrawNavInfo();
    virtual void       DrawNavPoint(Instruction& navpt, int index, int next);
    virtual void       DrawContactMarkers();
    virtual void       DrawContact(SimContact* c, int index);
    virtual void       DrawTrack(SimContact* c);
    virtual void       DrawTrackSegment(FVector& t1, FVector& t2, FColor c);
    virtual void       DrawRect(SimObject* targ);
    virtual void       DrawTarget();
    virtual void       DrawSight();
    virtual void       DrawLCOS(SimObject* targ, double dist);
    virtual void       DrawDesignators();
    virtual void       DrawFPM();
    virtual void       DrawHPM();
    virtual void       DrawCompass();
    virtual void       HideCompass();
    virtual void       DrawPitchLadder();
    virtual void       DrawStarSystem();

    virtual void       DrawMFDs();
    virtual void       DrawWarningPanel();
    virtual void       DrawInstructions();
    virtual void       DrawMessages();

    virtual void       MouseFrame();

    virtual int        GetHUDMode()      const { return mode; }
    virtual int        GetTacticalMode() const { return tactical; }
    virtual void       SetTacticalMode(int inMode = 1);
    virtual int        GetOverlayMode()  const { return overlay; }
    virtual void       SetOverlayMode(int inMode = 1);

    virtual void       SetHUDMode(int inMode);
    virtual void       CycleHUDMode();
    virtual FColor     CycleHUDColor();
    virtual void       SetHUDColorSet(int c);
    virtual int        GetHUDColorSet()  const { return color; }
    virtual FColor     GetHUDColor()     const { return hud_color; }
    virtual FColor     GetTextColor()    const { return txt_color; }
    virtual FColor     Ambient()         const;
    virtual void       ShowHUDWarn();
    virtual void       ShowHUDInst();
    virtual void       HideHUDWarn();
    virtual void       HideHUDInst();
    virtual void       CycleHUDWarn();
    virtual void       CycleHUDInst();
    virtual void       CycleMFDMode(int mfd);
    virtual void       CycleInstructions(int direction);
    virtual void       RestoreHUD();

    virtual void       TargetOff() { target = 0; }
    static  FColor     MarkerColor(SimContact* targ);

    static bool        IsNameCrowded(int x, int y);
    static bool        IsMouseLatched();
    static HUDView*    GetInstance() { return hud_view; }
    static void        Message(const char* fmt, ...);
    static void        ClearMessages();
    static void        PrepareBitmap(const char* name, Bitmap& img, BYTE*& shades);
    static void        TransferBitmap(const Bitmap& src, Bitmap& img, BYTE*& shades);
    static void        ColorizeBitmap(Bitmap& img, BYTE* shades, FColor color, bool force_alpha = false);

    static int         GetGunsight() { return gunsight; }
    static void        SetGunsight(int s) { gunsight = s; }
    static bool        IsArcade() { return arcade; }
    static void        SetArcade(bool a) { arcade = a; }
    static int         DefaultColorSet() { return def_color_set; }
    static void        SetDefaultColorSet(int c) { def_color_set = c; }
    static FColor      GetStatusColor(SimSystem::STATUS status);
    static bool        ShowFPS() { return show_fps; }
    static void        ShowFPS(bool f) { show_fps = f; }

    virtual bool         Update(SimObject* obj);
    virtual const char* GetObserverName() const;

protected:
    const char* FormatInstruction(Text instr);
    void              SetStatusColor(SimSystem::STATUS status);

    enum HUD_CASE { HUD_MIXED_CASE, HUD_UPPER_CASE };

    void              DrawDiamond(int x, int y, int r, FColor c);
    void              DrawHUDText(int index, const char* txt, Rect& rect, int align, int upcase = HUD_UPPER_CASE, bool box = false);
    void              HideHUDText(int index);

    void              DrawOrbitalBody(OrbitalBody* body);

    SimProjector* projector;
    CameraView* camview;

    int               width, height, aw, ah;
    double            xcenter, ycenter;

    Sim* sim;
    Ship* ship;
    SimObject* target;

    SimRegion* active_region;

    Bitmap* cockpit_hud_texture;

    FColor            hud_color;
    FColor            txt_color;
    FColor            status_color;

    bool              show_warn;
    bool              show_inst;
    int               inst_page;
    int               threat;

    int               mode;
    int               color;
    int               tactical;
    int               overlay;
    int               transition;
    int               docking;

    MFD* mfd[3];

    Sprite* pitch_ladder[31];
    Sprite* hud_sprite[32];

    Solid* az_ring;
    Solid* az_pointer;
    Solid* el_ring;
    Solid* el_pointer;
    double            compass_scale;

    enum { MAX_MSG = 6 };
    Text              msg_text[MAX_MSG];
    double            msg_time[MAX_MSG];

    static HUDView* hud_view;
    static bool       arcade;
    static bool       show_fps;
    static int        gunsight;
    static int        def_color_set;
};

// +--------------------------------------------------------------------+

struct HUDText
{
    SystemFont* font;
    FColor      color;
    Rect        rect;
    bool        hidden;
};
