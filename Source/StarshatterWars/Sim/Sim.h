/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright © 2025–2026. All Rights Reserved.

    ORIGINAL WORK:
    Starshatter 4.5
    Copyright © 1997–2004 Destroyer Studios LLC
    Original Author: John DiCamillo

    SUBSYSTEM:    StarshatterWars
    FILE:         Sim.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Simulation Universe and Region classes (UE-compliant, non-UObject)
*/

#pragma once
#include "Types.h"
#include "SimUniverse.h"  
#include "SimScene.h"
#include "Physical.h"
#include "List.h"
#include "Text.h"

// Minimal Unreal includes required for FVector / FColor:
#include "Math/Vector.h"
#include "Math/Color.h"

// +--------------------------------------------------------------------+

class Sim;
class SimRegion;
class SimObject;
class SimObserver;
class SimHyper;
class SimSplash;
class SimElement;
class SimSystem;

class SimContact;
class SimShot;

class StarSystem;
class Orbital;
class OrbitalRegion;
class Asteroid;

class CameraManager;

class Ship;
class ShipDesign;

class Drone;
class Explosion;
class Debris;
class WeaponDesign;
class MotionController;
class Dust;
class Grid;
class Mission;
class MissionElement;
class MissionEvent;
class Hangar;
class FlightDeck;

class Terrain;
class TerrainPatch;

class Model;

// +--------------------------------------------------------------------+

class Sim : public SimUniverse
{
    friend class SimRegion;

public:
    enum { REAL_SPACE, AIR_SPACE };

    Sim(MotionController* ctrl);
    virtual ~Sim();

    static Sim* GetSim() { return sim; }

    virtual void ExecFrame(double seconds);

    void LoadMission(Mission* msn, bool preload_textures = false);
    void ExecMission();
    void CommitMission();
    void UnloadMission();

    void NextView();
    void ShowGrid(int show = true);
    bool GridShown() const;

    const char* FindAvailCallsign(int IFF);
    SimElement* CreateElement(const char* callsign, int IFF, int type = 0);
    void DestroyElement(SimElement* elem);

    Ship* CreateShip(
        const char* name,
        const char* reg_num,
        ShipDesign* design,
        const char* rgn_name,
        const FVector& loc,
        int IFF = 0,
        int cmd_ai = 0,
        const int* loadout = nullptr
    );

    Ship* FindShip(const char* name, const char* rgn_name = nullptr);

    SimShot* CreateShot(
        const FVector& pos,
        const Camera& shot_cam,
        WeaponDesign* d,
        const Ship* ship = nullptr,
        SimRegion* rgn = nullptr
    );

    Explosion* CreateExplosion(
        const FVector& pos,
        const FVector& vel,
        int type,
        float exp_scale,
        float part_scale,
        SimRegion* rgn = nullptr,
        SimObject* source = nullptr,
        SimSystem* sys = nullptr
    );

    Debris* CreateDebris(const FVector& pos, const FVector& vel, Model* model, double mass, SimRegion* rgn = nullptr);
    Asteroid* CreateAsteroid(const FVector& pos, int type, double mass, SimRegion* rgn = nullptr);

    void CreateSplashDamage(Ship* ship);
    void CreateSplashDamage(SimShot* shot);
    void DestroyShip(Ship* ship);
    void NetDockShip(Ship* ship, Ship* carrier, FlightDeck* deck);

    virtual Ship* FindShipByObjID(uint32 objid);
    virtual SimShot* FindShotByObjID(uint32 objid);

    Mission* GetMission() { return mission; }
    List<MissionEvent>& GetEvents() { return events; }
    List<SimRegion>& GetRegions() { return regions; }

    SimRegion* FindRegion(const char* name);
    SimRegion* FindRegion(OrbitalRegion* rgn);
    SimRegion* FindNearestSpaceRegion(SimObject* object);
    SimRegion* FindNearestTerrainRegion(SimObject* object);
    SimRegion* FindNearestRegion(SimObject* object, int type);
    bool ActivateRegion(SimRegion* rgn);

    void RequestHyperJump(
        Ship* obj,
        SimRegion* rgn,
        const FVector& loc,
        int type = 0,
        Ship* fc_src = nullptr,
        Ship* fc_dst = nullptr
    );

    SimRegion* GetActiveRegion() { return active_region; }
    StarSystem* GetStarSystem() { return star_system; }
    SimScene* GetScene() { return scene; }

    Ship* GetPlayerShip();
    SimElement* GetPlayerElement();
    Orbital* FindOrbitalBody(const char* name);

    void SetSelection(Ship* s);
    bool IsSelected(Ship* s);
    ListIter<Ship> GetSelection();
    void ClearSelection();
    void AddSelection(Ship* s);

    void SetTestMode(bool t = true);

    bool IsTestMode() const { return test_mode; }
    bool IsActive()   const;
    bool IsComplete() const;

    MotionController* GetControls() const { return ctrl; }

    SimElement* FindElement(const char* name);
    int GetAssignedElements(SimElement* elem, List<SimElement>& assigned);
    List<SimElement>& GetElements() { return elements; }

    void SkipCutscene();
    void ResolveTimeSkip(double seconds);
    void ResolveHyperList();
    void ResolveSplashList();

    void ExecEvents(double seconds);
    void ProcessEventTrigger(int type, int event_id = 0, const char* ship = nullptr, int param = 0);

    double MissionClock() const;
    uint32 StartTime() const { return start_time; }

protected:
    void CreateRegions();
    void CreateElements();
    void CopyEvents();
    void BuildLinks();

    static Sim* sim;

    SimRegion* active_region;
    StarSystem* star_system;
    SimScene*   scene;
    Dust* dust;
    CameraManager* cam_dir;

    List<SimRegion>      regions;
    List<SimRegion>      rgn_queue;
    List<SimHyper>       jumplist;
    List<SimSplash>      splashlist;
    List<SimElement>     elements;
    List<SimElement>     finished;
    List<MissionEvent>   events;
    List<MissionElement> mission_elements;

    MotionController* ctrl;

    bool        test_mode;
    bool        grid_shown;
    Mission*    mission;
    uint32      start_time;
};

// +--------------------------------------------------------------------+

class SimRegion
{
    friend class Sim;

public:
    static const char* TYPENAME() { return "SimRegion"; }

    enum { REAL_SPACE, AIR_SPACE };

    SimRegion(Sim* sim, const char* name, int type);
    SimRegion(Sim* sim, OrbitalRegion* rgn);
    virtual ~SimRegion();

    int operator == (const SimRegion& r) const { return (sim == r.sim) && (name == r.name); }
    int operator <  (const SimRegion& r) const;
    int operator <= (const SimRegion& r) const;

    virtual void         Activate();
    virtual void         Deactivate();
    virtual void         ExecFrame(double seconds);
    void                 ShowGrid(int show = true);
    void                NextView();
    Ship*               FindShip(const char* name);
    Ship*               GetPlayerShip() { return player_ship; }
    void                SetPlayerShip(Ship* ship);
    OrbitalRegion*      GetOrbitalRegion() { return orbital_region; }
    Terrain*            GetTerrain() { return terrain; }
    bool                 IsActive()   const { return active; }
    bool                 IsAirSpace() const { return type == AIR_SPACE; }
    bool                 IsOrbital()  const { return type == REAL_SPACE; }
    bool                 CanTimeSkip()const;

    virtual Ship*          FindShipByObjID(DWORD objid);
    virtual SimShot*       FindShotByObjID(DWORD objid);

    virtual void         InsertObject(Ship* ship);
    virtual void         InsertObject(SimShot* shot);
    virtual void         InsertObject(Explosion* explosion);
    virtual void         InsertObject(Debris* debris);
    virtual void         InsertObject(Asteroid* asteroid);

    const char*             GetName() const { return name; }
    int                     GetType() const { return type; }
    int                     GetNumShips() { return ships.size(); }
    List<Ship>&             GetShips() { return ships; }
    List<Ship>&             GetCarriers() { return carriers; }
    List<SimShot>&          GetShots() { return shots; }
    List<Drone>&            GetDrones() { return drones; }
    List<Debris>&           GetRocks() { return debris; }
    List<Asteroid>&         GetRoids() { return asteroids; }
    List<Explosion>&        GetExplosions() { return explosions; }
    List<SimRegion>&        GetLinks() { return links; }
    StarSystem*             GetSystem() { return star_system; }

    Point                Location() const { return location; }

    void                 SetSelection(Ship* s);
    bool                 IsSelected(Ship* s);
    ListIter<Ship>       GetSelection();
    void                 ClearSelection();
    void                 AddSelection(Ship* s);

    List<SimContact>& TrackList(int iff);

    void                 ResolveTimeSkip(double seconds);

protected:
    void                 CommitMission();
    void                 TranslateObject(SimObject* object);

    void                 AttachPlayerShip(int index);
    void                 DestroyShips();
    void                 DestroyShip(Ship* ship);
    void                 NetDockShip(Ship* ship, Ship* carrier, FlightDeck* deck);

    void                 UpdateSky(double seconds, const Point& ref);
    void                 UpdateShips(double seconds);
    void                 UpdateShots(double seconds);
    void                 UpdateExplosions(double seconds);
    void                 UpdateTracks(double seconds);

    void                 DamageShips();
    void                 CollideShips();
    void                 CrashShips();
    void                 DockShips();

    Sim* sim;
    Text                 name;
    int                  type;
    StarSystem* star_system;
    OrbitalRegion* orbital_region;
    Point                location;
    Grid* grid;
    Terrain* terrain;
    bool                 active;

    Ship* player_ship;
    int                  current_view;
    List<Ship>           ships;
    List<Ship>           carriers;
    List<Ship>           selection;
    List<Ship>           dead_ships;
    List<SimShot>        shots;
    List<Drone>          drones;
    List<Explosion>      explosions;
    List<Debris>         debris;
    List<Asteroid>       asteroids;
    List<SimContact>     track_database[5];
    List<SimRegion>      links;

    DWORD                sim_time;
    int                  ai_index;
};