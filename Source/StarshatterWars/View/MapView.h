/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe (legacy) -> Starshatter Wars (UE port)
    FILE:         MapView.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    MapView (UE port, plain C++ class)
    - Tactical/strategic map rendering and interaction
    - Keeps Starshatter core container/string types (Text, List, etc.)
    - Uses Unreal math types for vectors (FVector) and colors (FColor)
*/

#pragma once

#include "Math/Vector.h"               // FVector
#include "Math/Color.h"                // FColor
#include "Math/UnrealMathUtility.h"    // Math

// Minimal project includes (keep header light):
#include "View.h"
#include "List.h"
#include "Text.h"
#include "SimObject.h"
#include "EventTarget.h"

// Forward declarations (aggressive):
class ActiveWindow;
class Campaign;
class Combatant;
class CombatGroup;
class CombatUnit;
class Galaxy;
class Instruction;
class Menu;
class MenuView;
class Mission;
class MissionElement;
class Orbital;
class OrbitalBody;
class OrbitalRegion;
class Ship;
class Sim;
class SimElement;
class SimLight;
class SimProjector;
class SimScene;
class SimShot;
class SimSystem;
class Star;
class StarSystem;
class Window;
class Bitmap;

// NOTE: User mapping requested "Contact* should by SimCo0ntact*"
class SimContact;

// Requested renames:
class CameraManager;   // was CameraDirector
class MusicManager;    // was MusicDirector
class SystemFont;      // was Font

#include "GameStructs.h" // last include

// +--------------------------------------------------------------------+

class MapView : public View, public SimObserver, public EventTarget
{
public:
    MapView(View* InParent, ActiveWindow* InActiveWindow);
    virtual ~MapView();

    // View:
    virtual void      Refresh() override;
    virtual void      OnWindowMove() override;

    // Visibility lifecycle:
    virtual void      OnShow();
    virtual void      OnHide();

    // Input:
    virtual int       OnMouseMove(int x, int y);
    virtual int       OnRButtonDown(int x, int y);
    virtual int       OnRButtonUp(int x, int y);
    virtual int       OnLButtonDown(int x, int y);
    virtual int       OnLButtonUp(int x, int y);
    virtual int       OnClick();

    // Data binding:
    void              SetGalaxy(List<StarSystem>& g);
    void              SetSystem(StarSystem* s);
    void              SetShip(Ship* s);
    void              SetMission(Mission* m);
    void              SetCampaign(Campaign* cpn);

    // Modes:
    int               GetViewMode() const;
    void              SetViewMode(int mode);

    void              SetSelectionMode(int mode);
    int               GetSelectionMode() const;

    // Selection:
    void              SetSelection(int index);
    void              SetSelectedShip(Ship* sel);
    void              SetSelectedElem(MissionElement* elem);

    Orbital* GetSelection();
    int               GetSelectionIndex();

    Ship* GetSelectedShip();
    MissionElement* GetSelectedElem();

    // Region:
    void              SetRegion(OrbitalRegion* rgn);
    void              SetRegionByName(const char* rgn_name);
    OrbitalRegion* GetRegion() const;

    // Zoom:
    void              ZoomIn();
    void              ZoomOut();

    // Sim observer:
    virtual bool      Update(SimObject* obj) override;

protected:
    // Internals:
    void              BuildMenu();
    void              ClearMenu();
    void              ProcessMenuItem(int action);

    // Capture:
    bool              SetCapture();
    bool              ReleaseCapture();

    // Scrolling/visibility:
    void              SetupScroll(Orbital* s);
    bool              IsVisible(const FVector& loc);

    // Picking:
    void              SelectAt(int x, int y);

    // Ship/element/nav selection helpers:
    void              SelectShip(Ship* selship);
    void              SelectElem(MissionElement* elem);
    void              SelectNavpt(Instruction* navpt);

    // Objective population:
    void              FindShips(bool friendly, bool station, bool starship, bool dropship, List<Text>& result);

    // Drawing:
    void              DrawTitle();
    void              DrawTabbedText(SystemFont* InFont, const char* text);

    void              DrawGrid();
    void              DrawGalaxy();
    void              DrawSystem();
    void              DrawRegion();

    void              DrawOrbital(Orbital& body, int index);
    double            GetMinRadius(int type);

    // System overlay (campaign):
    void              DrawCombatantSystem(Combatant* cbt, Orbital* rgn, int x, int y, int rr);
    void              DrawCombatGroupSystem(CombatGroup* group, Orbital* rgn, int x1, int x2, int& y, int align);

    // Region overlay:
    void              DrawCombatGroup(CombatGroup* group, int rep);
    void              DrawShip(Ship& s, bool current, int rep);
    void              DrawElem(MissionElement& s, bool current, int rep);

    // Routes:
    void              DrawNavRoute(OrbitalRegion* rgn, List<Instruction>& s_route, FColor s_marker, Ship* ship, MissionElement* elem);

    // Crowding/clutter:
    bool              IsClutter(Ship& test);
    bool              IsCrowded(Ship& test);
    bool              IsCrowded(MissionElement& test);

    bool              IsEnabled() const;
    bool              IsVisible() const;

    bool              IsFormActive() const;
    Rect              TargetRect() const;

    // Coordinate helpers:
    void              GetShipLoc(Ship& s, FVector& shiploc);
    void              GetElemLoc(MissionElement& s, FVector& shiploc);

protected:
    // -----------------------------------------------------------------
    // State
    // -----------------------------------------------------------------

    Text              title;
    Rect              rect;

    Campaign* campaign;
    Mission* mission;

    List<StarSystem>  system_list;
    StarSystem* system;

    List<Orbital>     stars;
    List<Orbital>     planets;
    List<Orbital>     regions;

    Ship* ship;

    Bitmap*           galaxy_image = nullptr;

    bool              editor;

    int               current_star;
    int               current_planet;
    int               current_region;

    Ship* current_ship;
    MissionElement* current_elem;
    Instruction* current_navpt;

    int               current_status;

    int               view_mode;
    int               seln_mode;

    bool              captured;
    bool              dragging;
    bool              adding_navpt;
    bool              moving_navpt;
    bool              moving_elem;

    int               scrolling;
    int               mouse_x;
    int               mouse_y;

    unsigned int      ship_filter;

    double            zoom;
    double            offset_x;
    double            offset_y;

    // Cached scale helpers (legacy naming kept):
    double            c;
    double            r;

    // Smooth scrolling:
    double            scroll_x;
    double            scroll_y;

    // Click world coords (region view):
    double            click_x;
    double            click_y;

    // Per-view saved zoom/offset:
    double            view_zoom[3];
    double            view_offset_x[3];
    double            view_offset_y[3];

    // Window/menu:
    ActiveWindow* active_window;
    Menu* active_menu;

    Menu* map_menu;
    Menu* map_system_menu;
    Menu* map_sector_menu;

    Menu* ship_menu;
    Menu* nav_menu;

    Menu* action_menu;
    Menu* objective_menu;
    Menu* formation_menu;
    Menu* speed_menu;
    Menu* hold_menu;
    Menu* farcast_menu;

    MenuView* menu_view;

    const int EID_MAP_CLICK = 1000;
};
