/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (C) 2025–2026.

    SUBSYSTEM:    Stars.exe (Unreal Port)
    FILE:         HUDView.h
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    Heads-Up Display view (Unreal UUserWidget-based)

    NOTES
    =====
    - Inherits from UView (UUserWidget)
    - Pure Unreal C++ (no Blueprint logic)
    - Legacy Starshatter HUD API preserved
    - Uses Slate rendering via NativePaint (in .cpp)
    - UE-native math and color types internally
*/

#pragma once

#include "CoreMinimal.h"

// Unreal:
#include "Math/Vector.h"                // FVector
#include "Math/Color.h"                 // FColor
#include "Engine/Font.h"                // UFont

// Project / Starshatter:
#include "Types.h"
#include "GameStructs.h"
#include "View.h"    
#include "MFDView.h" // UView
#include "Bitmap.h"
#include "SimSystem.h"
#include "SimObject.h"
#include "Text.h"
#include "SystemFont.h"
#include "GameStructs.h"

// +--------------------------------------------------------------------+
// Forward declarations

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
class UMFDView;
class Sim;
class SimRegion;
class Window;

// +--------------------------------------------------------------------+

class UHUDView : public UView, public SimObserver
{
public:
    UHUDView(Window* c);
    virtual ~UHUDView();


    // ----------------------------------------------------------------
    // Core lifecycle / control
    // ----------------------------------------------------------------
    virtual void        Refresh() override;
    virtual void        OnWindowMove() override;
    virtual void        ExecFrame(float DeltaTime);

    virtual void        UseCameraView(CameraView* v);


    // ----------------------------------------------------------------
    // State access
    // ----------------------------------------------------------------
    virtual Ship* GetShip()   const { return ship; }
    virtual SimObject* GetTarget() const { return target; }

    virtual void        SetShip(Ship* s);
    virtual void        SetTarget(SimObject* t);

    virtual UMFDView* GetMFD(int n) const;

    // ----------------------------------------------------------------
    // Drawing pipeline (legacy API preserved)
    // ----------------------------------------------------------------
    virtual void        HideAll();

    virtual void        DrawBars();
    virtual void        DrawNav();
    virtual void        DrawILS();
    virtual void        DrawObjective();
    virtual void        DrawNavInfo();
    virtual void        DrawNavPoint(Instruction& navpt, int index, int next);

    virtual void        DrawContactMarkers();
    virtual void        DrawContact(SimContact* c, int index);
    virtual void        DrawTrack(SimContact* c);
    virtual void        DrawTrackSegment(const FVector& t1, const FVector& t2, FColor c);

    virtual void        DrawRect(SimObject* targ);
    virtual void        DrawTarget();
    virtual void        DrawSight();
    virtual void        DrawLCOS(SimObject* targ, double dist);

    virtual void        DrawDesignators();
    virtual void        DrawFPM();
    virtual void        DrawHPM();

    virtual void        DrawCompass();
    virtual void        HideCompass();
    virtual void        DrawPitchLadder();
    virtual void        DrawStarSystem();

    virtual void        DrawMFDs();
    virtual void        DrawWarningPanel();
    virtual void        DrawInstructions();
    virtual void        DrawMessages();

    virtual void        MouseFrame();

    // ----------------------------------------------------------------
    // Modes / configuration
    // ----------------------------------------------------------------
    virtual int         GetHUDMode()      const { return mode; }
    virtual int         GetTacticalMode() const { return tactical; }
    virtual int         GetOverlayMode()  const { return overlay; }

    virtual void        SetTacticalMode(int mode = 1);
    virtual void        SetOverlayMode(int mode = 1);

    virtual void        SetHUDMode(int mode);
    virtual void        CycleHUDMode();

    virtual FColor      CycleHUDColor();
    virtual void        SetHUDColorSet(int c);

    virtual int         GetHUDColorSet()  const { return color; }
    virtual FColor      GetHUDColor()     const { return hud_color; }
    virtual FColor      GetTextColor()    const { return txt_color; }
    virtual FColor      Ambient()         const;

    virtual void        ShowHUDWarn();
    virtual void        HideHUDWarn();
    virtual void        CycleHUDWarn();

    virtual void        ShowHUDInst();
    virtual void        HideHUDInst();
    virtual void        CycleHUDInst();

    virtual void        CycleMFDMode(int mfd);
    virtual void        CycleInstructions(int direction);
    virtual void        RestoreHUD();

    // ----------------------------------------------------------------
    // Target / contact helpers
    // ----------------------------------------------------------------
    virtual void        TargetOff() { target = nullptr; }

    static  FColor      MarkerColor(SimContact* targ);

    static bool         IsNameCrowded(int x, int y);
    static bool         IsMouseLatched();

    // ----------------------------------------------------------------
    // Global HUD utilities
    // ----------------------------------------------------------------
    static UHUDView* GetInstance() { return hud_view; }

    static void         Message(const char* fmt, ...);
    static void         ClearMessages();

    static void         PrepareBitmap(const char* name, Bitmap& img, BYTE*& shades);
    static void         TransferBitmap(const Bitmap& src, Bitmap& img, BYTE*& shades);
    static void         ColorizeBitmap(
        Bitmap& img,
        BYTE* shades,
        FColor color,
        bool force_alpha = false);

    static int          GetGunsight() { return gunsight; }
    static void         SetGunsight(int s) { gunsight = s; }

    static bool         IsArcade() { return arcade; }
    static void         SetArcade(bool a) { arcade = a; }

    static int          DefaultColorSet() { return def_color_set; }
    static void         SetDefaultColorSet(int c) { def_color_set = c; }

    static FColor       GetStatusColor(SYSTEM_STATUS status);

    static bool         ShowFPS() { return show_fps; }
    static void         ShowFPS(bool f) { show_fps = f; }

    // ----------------------------------------------------------------
    // SimObserver
    // ----------------------------------------------------------------
    virtual bool            Update(SimObject* obj) override;
    virtual const char* GetObserverName() const override;

    UPROPERTY()
    TArray<UMFDView*> MFDViews;

protected:
    // ----------------------------------------------------------------
    // Internal helpers
    // ----------------------------------------------------------------
    const char*         FormatInstruction(Text instr);
    void                SetStatusColor(SYSTEM_STATUS status);

    enum HUD_CASE
    {
        HUD_MIXED_CASE,
        HUD_UPPER_CASE
    };

    void                DrawDiamond(int x, int y, int r, FColor c);

    void                DrawHUDText(
        int index,
        const char* txt,
        Rect& rect,
        int align,
        int upcase = HUD_UPPER_CASE,
        bool box = false);

    void                HideHUDText(int index);

    void                DrawOrbitalBody(OrbitalBody* body);

protected:
    // ----------------------------------------------------------------
    // Core references
    // ----------------------------------------------------------------
    SimProjector* projector = nullptr;
    CameraView* camview = nullptr;

    Sim* sim = nullptr;
    Ship* ship = nullptr;
    SimObject* target = nullptr;

    SimRegion* active_region = nullptr;

    // ----------------------------------------------------------------
    // Geometry / layout
    // ----------------------------------------------------------------
    int                 width = 0;
    int                 height = 0;
    int                 aw = 0;
    int                 ah = 0;

    double              xcenter = 0.0;
    double              ycenter = 0.0;

    // ----------------------------------------------------------------
    // Visual state
    // ----------------------------------------------------------------
    Bitmap* cockpit_hud_texture = nullptr;

    FColor              hud_color;
    FColor              txt_color;
    FColor              status_color;

    bool                show_warn = false;
    bool                show_inst = false;

    int                 inst_page = 0;
    int                 threat = 0;

    int                 mode = (int)HUD_MODE::HUD_MODE_OFF;
    int                 color = 0;
    int                 tactical = 0;
    int                 overlay = 0;
    int                 transition = 0;
    int                 docking = 0;

    // ----------------------------------------------------------------
    // HUD components
    // ----------------------------------------------------------------
    UMFDView* mfd[3] = { nullptr, nullptr, nullptr };

    Sprite* pitch_ladder[31] = { nullptr };
    Sprite* hud_sprite[32] = { nullptr };

    Solid* az_ring = nullptr;
    Solid* az_pointer = nullptr;
    Solid* el_ring = nullptr;
    Solid* el_pointer = nullptr;

    double              compass_scale = 1.0;

    // ----------------------------------------------------------------
    // Messages
    // ----------------------------------------------------------------
    enum { MAX_MSG = 6 };

    Text                msg_text[MAX_MSG];
    double              msg_time[MAX_MSG] = { 0.0 };

    // ----------------------------------------------------------------
    // Fonts (Unreal)
    // ----------------------------------------------------------------
    SystemFont* hud_font = nullptr;
    SystemFont* msg_font = nullptr;

    // ----------------------------------------------------------------
    // Static HUD globals
    // ----------------------------------------------------------------
    static UHUDView* hud_view;

    static bool         arcade;
    static bool         show_fps;
    static int          gunsight;
    static int          def_color_set;
};

// +--------------------------------------------------------------------+

struct HUDText
{
    SystemFont* font = nullptr;
    FColor  color;
    Rect    rect;
    bool    hidden = false;
};
