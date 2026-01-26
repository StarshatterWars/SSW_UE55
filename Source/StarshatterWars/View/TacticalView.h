/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright 2025-2026. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         TacticalView.h
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR: John DiCamillo
    ORIGINAL STUDIO: Destroyer Studios LLC

    OVERVIEW
    ========
    View class for Radio Communications HUD Overlay
*/

#pragma once

#include "Types.h"
#include "View.h"
#include "SimObject.h"
#include "Text.h"

// Minimal Unreal include for FVector conversion (replaces Point/Vec3 usage in this header):
#include "Math/Vector.h" // FVector
#include "Math/Color.h"

// +--------------------------------------------------------------------+

class SystemFont;
class Ship;
class RadioMessage;
class CameraView;
class SimProjector;
class HUDView;
class Menu;
class MenuItem;
class MenuView;
class UGameScreen;

// +--------------------------------------------------------------------+

class TacticalView : public View, public SimObserver
{
public:
    TacticalView(Window* c, UGameScreen* parent);
    virtual ~TacticalView();

    // Operations:
    virtual void        Refresh();
    virtual void        OnWindowMove();
    virtual void        ExecFrame();
    virtual void        UseProjector(SimProjector* p);

    virtual void        DoMouseFrame();

    virtual bool        Update(SimObject* obj);
    virtual const char* GetObserverName() const;

    static void         SetColor(FColor c);

    static void         Initialize();
    static void         Close();

    static TacticalView* GetInstance() { return tac_view; }

protected:
    virtual bool        SelectAt(int x, int y);
    virtual bool        SelectRect(const Rect& r);
    virtual Ship*       WillSelectAt(int x, int y);
    virtual void        SetHelm(bool approach);

    virtual void        DrawMouseRect();
    virtual void        DrawSelection(Ship* seln);
    virtual void        DrawSelectionInfo(Ship* seln);
    virtual void        DrawSelectionList(ListIter<Ship> seln);

    virtual void        BuildMenu();
    virtual void        DrawMenu();
    virtual void        ProcessMenuItem(int action);

    virtual void        DrawMove();
    virtual void        SendMove();
    virtual bool        GetMouseLoc3D();

    virtual void        DrawAction();
    virtual void        SendAction();

    UGameScreen* gamescreen;
    CameraView* camview;
    SimProjector* projector;

    int                 width, height;
    double              xcenter, ycenter;

    int                 shift_down;
    int                 mouse_down;
    int                 right_down;
    int                 show_move;
    int                 show_action;

    FVector             move_loc;     // was: Point
    double              base_alt;
    double              move_alt;

    FVector             mouse_action;
    FVector             mouse_start;
    Rect                mouse_rect;

    SystemFont*         font;
    Sim*                sim;
    Ship*               ship;
    Ship*               msg_ship;
    Text                current_sector;

    Menu* active_menu;
    MenuView* menu_view;

    static TacticalView* tac_view;
};
