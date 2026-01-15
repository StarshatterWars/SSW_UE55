/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         Sim.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Simulation Universe and Region classes
*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Types.h"
#include "Universe.h"
//#include "Scene.h"
#include "Physical.h"
#include "Geometry.h"
#include "List.h"
#include "Text.h"
#include "Sim.generated.h"

// +--------------------------------------------------------------------+

class USim;
class SimRegion;
class USimObject;
class SimObserver;
class SimHyper;
class SimSplash;

class AStarSystem;
class AOrbital;
class AOrbitalRegion;
class Asteroid;

class NetGame;

class CameraDirector;
class Contact;
class UShip;
class ShipDesign;
class System;
class Element;
class Shot;
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

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API USim : public UUniverse
{
	GENERATED_BODY()
	
	friend class SimRegion;

public:
	enum { REAL_SPACE, AIR_SPACE };

	USim();
	USim(MotionController* ctrl);

	static USim*		 GetSim() { return sim; }

	//virtual void         ExecFrame(double seconds);

	void                 LoadMission(Mission* msn, bool preload_textures = false);
	void                 ExecMission();
	void                 CommitMission();
	void                 UnloadMission();

	void                 NextView();
	void                 ShowGrid(int show = true);
	bool                 GridShown() const;

	const char*			 FindAvailCallsign(int IFF);
	Element*			 CreateElement(const char* callsign, int IFF, int type = 0/*PATROL*/);
	void                 DestroyElement(Element* elem);
	UShip*				 CreateShip(const char* name,
									const char* reg_num,
									ShipDesign* design,
									const char* rgn_name,
									const Point& loc,
									int         IFF = 0,
									int         cmd_ai = 0,
									const int* loadout = 0);

	UShip*				 FindShip(const char* name, const char* rgn_name = 0);
	Shot*				 CreateShot(const Point& pos, 
									const UCamera& shot_cam,
									WeaponDesign* d, 
									const UShip* ship = 0,
									SimRegion* rgn = 0);

	Explosion*			 CreateExplosion(const Point& pos,
										 const Point& vel, 
										 int type, 
										 float exp_scale, 
										 float part_scale, 
										 SimRegion* rgn = 0, 
										 USimObject* source = 0, 
										 System* sys = 0);

	Debris*				 CreateDebris(const Point& pos, 
									  const Point& vel, 
									  Model* model, 
									  double mass, 
									  SimRegion* rgn = 0);

	Asteroid*			 CreateAsteroid(const Point& pos,
										int type, 
										double mass, 
										SimRegion* rgn = 0);

	void                 CreateSplashDamage(UShip* ship);
	void                 CreateSplashDamage(Shot* shot);
	void                 DestroyShip(UShip* ship);
	void                 NetDockShip(UShip* ship, UShip* carrier, FlightDeck* deck);

	UShip*				 FindShipByObjID(DWORD objid);
	Shot*				 FindShotByObjID(DWORD objid);

	Mission* GetMission() { return mission; }
	List<MissionEvent>& GetEvents() { return events; }
	List<SimRegion>& GetRegions() { return regions; }
	SimRegion* FindRegion(const char* name);
	SimRegion* FindRegion(AOrbitalRegion* rgn);
	SimRegion* FindNearestSpaceRegion(USimObject* object);
	SimRegion* FindNearestSpaceRegion(AOrbital* body);
	SimRegion* FindNearestTerrainRegion(USimObject* object);
	SimRegion* FindNearestRegion(USimObject* object, int type);
	bool                 ActivateRegion(SimRegion* rgn);

	void                 RequestHyperJump(UShip* obj,
		SimRegion* rgn,
		const Point& loc,
		int          type = 0,
		UShip* fc_src = 0,
		UShip* fc_dst = 0);

	SimRegion* GetActiveRegion() { return active_region; }
	AStarSystem* GetStarSystem() { return star_system; }
	List<AStarSystem>& GetSystemList();
	//Scene* GetScene() { return scene; }
	UShip* GetPlayerShip();
	Element* GetPlayerElement();
	AOrbital* FindOrbitalBody(const char* name);

	void                 SetSelection(UShip* s);
	bool                 IsSelected(UShip* s);
	ListIter<UShip>       GetSelection();
	void                 ClearSelection();
	void                 AddSelection(UShip* s);

	void                 SetTestMode(bool t = true);

	bool                 IsTestMode()   const { return test_mode; }
	bool                 IsNetGame()    const { return netgame != 0; }
	bool                 IsActive()     const;
	bool                 IsComplete()   const;

	MotionController* GetControls()  const { return ctrl; }

	Element* FindElement(const char* name);
	List<Element>& GetElements() { return elements; }

	int                  GetAssignedElements(Element* elem, List<Element>& assigned);

	void                 SkipCutscene();
	void                 ResolveTimeSkip(double seconds);
	void                 ResolveHyperList();
	void                 ResolveSplashList();

	void                 ExecEvents(double seconds);
	void                 ProcessEventTrigger(int type, int event_id = 0, const char* ship = 0, int param = 0);
	double               MissionClock() const;
	DWORD                StartTime()    const { return start_time; }

	// Create a list of mission elements based on the current
	// state of the simulation.  Used for multiplayer join in progress.
	ListIter<MissionElement> GetMissionElements();
	static USim* sim;

protected:
	void                 CreateRegions();
	void                 CreateElements();
	void                 CopyEvents();
	void                 BuildLinks();

	// Convert a single live element into a mission element
	// that can be serialized over the net.
	MissionElement* CreateMissionElement(Element* elem);
	Hangar* FindSquadron(const char* name, int& index);

	
	SimRegion* active_region;
	AStarSystem* star_system;
	//Scene*  scene;
	Dust* dust;
	CameraDirector* cam_dir;

	List<SimRegion>      regions;
	List<SimRegion>      rgn_queue;
	List<SimHyper>       jumplist;
	List<SimSplash>      splashlist;
	List<Element>        elements;
	List<Element>        finished;
	List<MissionEvent>   events;
	List<MissionElement> mission_elements;

	MotionController* ctrl;

	bool                 test_mode;
	bool                 grid_shown;
	Mission* mission;

	NetGame* netgame;
	DWORD                start_time;
};

// +--------------------------------------------------------------------+

class SimRegion
{
	friend class USim;

public:
	static const char* TYPENAME() { return "SimRegion"; }

	enum { REAL_SPACE, AIR_SPACE };

	SimRegion(USim* sim, const char* name, int type);
	SimRegion(USim* sim, AOrbitalRegion* rgn);
	virtual ~SimRegion();

	int operator == (const SimRegion& r) const { return (sim == r.sim) && (name == r.name); }
	int operator <  (const SimRegion& r) const;
	int operator <= (const SimRegion& r) const;

	virtual void         Activate();
	virtual void         Deactivate();
	virtual void         ExecFrame(double seconds);
	void                 ShowGrid(int show = true);
	void                 NextView();
	UShip* FindShip(const char* name);
	UShip* GetPlayerShip() { return player_ship; }
	void                 SetPlayerShip(UShip* ship);
	AOrbitalRegion* GetOrbitalRegion() { return orbital_region; }
	Terrain* GetTerrain() { return terrain; }
	bool                 IsActive()   const { return active; }
	bool                 IsAirSpace() const { return type == AIR_SPACE; }
	bool                 IsOrbital()  const { return type == REAL_SPACE; }
	bool                 CanTimeSkip()const;

	UShip* FindShipByObjID(DWORD objid);
	virtual Shot* FindShotByObjID(DWORD objid);

	virtual void         InsertObject(UShip* ship);
	virtual void         InsertObject(Shot* shot);
	virtual void         InsertObject(Explosion* explosion);
	virtual void         InsertObject(Debris* debris);
	virtual void         InsertObject(Asteroid* asteroid);

	const char* Name() const { return name; }
	int                  Type() const { return type; }
	int                  NumShips() { return ships.size(); }
	List<UShip>& Ships() { return ships; }
	List<UShip>& Carriers() { return carriers; }
	List<Shot>& Shots() { return shots; }
	List<Drone>& Drones() { return drones; }
	List<Debris>& Rocks() { return debris; }
	List<Asteroid>& Roids() { return asteroids; }
	List<Explosion>& Explosions() { return explosions; }
	List<SimRegion>& Links() { return links; }
	AStarSystem* System() { return star_system; }

	Point                Location() const { return location; }

	void                 SetSelection(UShip* s);
	bool                 IsSelected(UShip* s);
	ListIter<UShip>       GetSelection();
	void                 ClearSelection();
	void                 AddSelection(UShip* s);

	List<Contact>& TrackList(int iff);

	void                 ResolveTimeSkip(double seconds);
	USim* sim;

protected:
	void                 CommitMission();
	void                 TranslateObject(USimObject* object);

	void                 AttachPlayerShip(int index);
	void                 DestroyShips();
	void                 DestroyShip(UShip* ship);
	void                 NetDockShip(UShip* ship, UShip* carrier, FlightDeck* deck);

	void                 UpdateSky(double seconds, const Point& ref);
	void                 UpdateShips(double seconds);
	void                 UpdateShots(double seconds);
	void                 UpdateExplosions(double seconds);
	void                 UpdateTracks(double seconds);

	void                 DamageShips();
	void                 CollideShips();
	void                 CrashShips();
	void                 DockShips();


	Text                 name;
	int                  type;
	AStarSystem* star_system;
	AOrbitalRegion* orbital_region;
	Point                location;
	Grid* grid;
	Terrain* terrain;
	bool                 active;

	UShip* player_ship;
	int                  current_view;
	List<UShip>           ships;
	List<UShip>           carriers;
	List<UShip>           selection;
	List<UShip>           dead_ships;
	List<Shot>           shots;
	List<Drone>          drones;
	List<Explosion>      explosions;
	List<Debris>         debris;
	List<Asteroid>       asteroids;
	List<Contact>        track_database[5];
	List<SimRegion>      links;

	DWORD                sim_time;
	int                  ai_index;
};

// +--------------------------------------------------------------------+

class SimHyper
{
public:
	SimHyper(UShip* o, SimRegion* r, const Point& l, int t, bool h, UShip* fc1, UShip* fc2);

		UShip*		ship;
		SimRegion*	rgn;
		Point       loc;
		int         type;
		bool        hyperdrive;
		UShip*		fc_src;
		UShip*		fc_dst;
};

// +--------------------------------------------------------------------+

class SimSplash
{
public:
	SimSplash(SimRegion* r, const Point& l, double d, double n);
		
	Text        owner_name;
	Point       loc;
	double      damage;
	double      range;
	SimRegion* rgn;
	bool        missile;
};


