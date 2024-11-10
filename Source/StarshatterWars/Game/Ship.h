/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         Ship.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Starship (or space/ground station) class
*/

#pragma once

#include "CoreMinimal.h"
#include "SimObject.h"
#include "SimObject.h"
//#include "DetailSet.h"
//#include "Director.h"
#include "../Foundation/Types.h"
#include "../Foundation/Geometry.h"
#include "../Foundation/Color.h"
#include "../Foundation/List.h"
#include "Ship.generated.h"


// +--------------------------------------------------------------------+

class UShip;
class Shot;
class Drone;

class Bitmap;
class CombatUnit;
class Computer;
class Contact;
class Drive;
class Element;
class Farcaster;
class FlightComp;
class FlightDeck;
class Hangar;
class InboundSlot;
class Instruction;
class LandingGear;
class MotionController;
class NavLight;
class NavSystem;
class PowerSource;
class QuantumDrive;
class RadioMessage;
class Shield;
class ShieldRep;
class ShipDesign;
class ShipKiller;
class Solid;
//class Skin;
class Sound;
class Sensor;
class System;
class Thruster;
class Weapon;
class WeaponDesign;
class WeaponGroup;

// +--------------------------------------------------------------------+

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UShip : public USimObject, public SimObserver
{
	GENERATED_BODY()

public:	
	static const char* TYPENAME() { return "Ship"; }

	enum CLASSIFICATION {
		DRONE = 0x0001,
		FIGHTER = 0x0002,
		ATTACK = 0x0004,
		LCA = 0x0008,
		COURIER = 0x0010,
		CARGO = 0x0020,
		CORVETTE = 0x0040,
		FREIGHTER = 0x0080,
		FRIGATE = 0x0100,
		DESTROYER = 0x0200,
		CRUISER = 0x0400,
		BATTLESHIP = 0x0800,
		CARRIER = 0x1000,
		DREADNAUGHT = 0x2000,

		STATION = 0x4000,
		FARCASTER = 0x8000,

		MINE = 0x00010000,
		COMSAT = 0x00020000,
		DEFSAT = 0x00040000,
		SWACS = 0x00080000,

		BUILDING = 0x00100000,
		FACTORY = 0x00200000,
		SAM = 0x00400000,
		EWR = 0x00800000,
		C3I = 0x01000000,
		STARBASE = 0x02000000,

		DROPSHIPS = 0x0000000f,
		STARSHIPS = 0x0000fff0,
		SPACE_UNITS = 0x000f0000,
		GROUND_UNITS = 0xfff00000
	};

	enum OP_MODE { DOCKED, ALERT, LOCKED, LAUNCH, TAKEOFF, ACTIVE, APPROACH, RECOVERY, DOCKING };
	enum FLCS_MODE { FLCS_MANUAL, FLCS_AUTO, FLCS_HELM };
	enum TRAN_TYPE {
		TRANSITION_NONE,
		TRANSITION_DROP_CAM,
		TRANSITION_DROP_ORBIT,
		TRANSITION_MAKE_ORBIT,
		TRANSITION_TIME_SKIP,
		TRANSITION_DEATH_SPIRAL,
		TRANSITION_DEAD
	};

	enum FLIGHT_MODEL { FM_STANDARD, FM_RELAXED, FM_ARCADE };
	enum LANDING_MODEL { LM_STANDARD, LM_EASIER };
	
	// CONSTRUCTORS:
	UShip();
	UShip(const char* ship_name, const char* reg_num, ShipDesign* design, int IFF = 0, int cmd_ai = 0, const int* loadout = 0);

	int operator == (const UShip& s) const { return id == s.id; }

	static void       Initialize();
	static void       Close();

	void      ExecFrame(double seconds);
	void      AeroFrame(double seconds);
	void      StatFrame(double seconds);
	void      DockFrame(double seconds);
	void      LinearFrame(double seconds);
	void      ExecSensors(double seconds);

	void      ExecNavFrame(double seconds);
	void      ExecPhysics(double seconds);
	void      ExecThrottle(double seconds);
	void      ExecSystems(double seconds);

	void      Activate(Scene& scene);
	void      Deactivate(Scene& scene);
	void      SelectDetail(double seconds);
	void      SetRegion(SimRegion* rgn);
	int       GetTextureList(List<Bitmap>& textures);

	// DIRECTION:
	void			  SetControls(MotionController* m);
	void			  SetNetworkControl(Director* net_ctrl = 0);
	void              SetDirectorInfo(const char* msg) { director_info = msg; }
	const char*		  GetDirectorInfo()          const { return director_info; }
	void              SetAIMode(int n) { ai_mode = (BYTE)n; }
	int               GetAIMode()                const { return (int)ai_mode; }
	void              SetCommandAILevel(int n) { command_ai_level = (BYTE)n; }
	int               GetCommandAILevel()        const { return command_ai_level; }
	int       GetFlightPhase()           const { return flight_phase; }
	void      SetFlightPhase(OP_MODE phase);
	bool              IsNetObserver()            const { return net_observer_mode; }
	void              SetNetObserver(bool n) { net_observer_mode = n; }

	bool              IsInvulnerable()           const { return invulnerable; }
	void              SetInvulnerable(bool n) { invulnerable = n; }

	double            GetHelmHeading()     const { return helm_heading; }
	double            GetHelmPitch()       const { return helm_pitch; }
	void              SetHelmHeading(double h);
	void              SetHelmPitch(double p);
	void      ApplyHelmYaw(double y);
	void      ApplyHelmPitch(double p);
	void      ApplyPitch(double pitch_acc); // override for G limiter

	void              ArcadeStop() { arcade_velocity *= 0; }

	// CAMERA:
	Point             BridgeLocation()     const { return bridge_vec; }
	Point             ChaseLocation()      const { return chase_vec; }
	Point             TransitionLocation() const { return transition_loc; }

	// FLIGHT DECK:
	UShip* GetController()    const;
	int               NumInbound()       const;
	int               NumFlightDecks()   const;
	FlightDeck* GetFlightDeck(int i = 0) const;
	UShip* GetCarrier()       const { return carrier; }
	FlightDeck* GetDock()          const { return dock; }
	void              SetCarrier(UShip* c, FlightDeck* d);
	void              Stow();
	InboundSlot* GetInbound()      const { return inbound; }
	void              SetInbound(InboundSlot* s);

	// DRIVE SYSTEMS:
	int               GetFuelLevel()       const; // (0-100) percent of full tank
	void              SetThrottle(double percent);
	void              SetAugmenter(bool  enable);
	double            Thrust(double seconds) const;
	double            VelocityLimit()      const { return vlimit; }
	Drive* GetDrive()           const { return main_drive; }
	double            Throttle()           const { return throttle; }
	bool              Augmenter()          const { return augmenter; }
	QuantumDrive* GetQuantumDrive()    const { return quantum_drive; }
	Farcaster* GetFarcaster()       const { return farcaster; }

	bool              IsAirborne()         const;
	bool              IsDropCam()          const { return transition_type == TRANSITION_DROP_CAM; }
	bool              IsDropping()         const { return transition_type == TRANSITION_DROP_ORBIT; }
	bool              IsAttaining()        const { return transition_type == TRANSITION_MAKE_ORBIT; }
	bool              IsSkipping()         const { return transition_type == TRANSITION_TIME_SKIP; }
	bool              IsDying()            const { return transition_type == TRANSITION_DEATH_SPIRAL; }
	bool              IsDead()             const { return transition_type == TRANSITION_DEAD; }
	bool              InTransition()       const { return transition_type != TRANSITION_NONE; }
	void              DropOrbit();
	void              MakeOrbit();
	bool              CanTimeSkip();
	bool              IsInCombat();
	void              TimeSkip();
	void              DropCam(double time = 10, double range = 0);
	void              DeathSpiral();
	void              CompleteTransition();
	void              SetTransition(double trans_time, int trans_type, const Point& trans_loc);

	double            CompassHeading()     const;
	double            CompassPitch()       const;
	double            AltitudeMSL()        const;
	double            AltitudeAGL()        const;
	double            GForce()             const;

	void			  SetupAgility();

	// FLIGHT CONTROL SYSTEM (FLCS):
	void              ExecFLCSFrame();
	void              CycleFLCSMode();
	void              SetFLCSMode(int mode);
	int               GetFLCSMode() const;
	void              SetTransX(double t);
	void              SetTransY(double t);
	void              SetTransZ(double t);

	bool              IsGearDown();
	void              LowerGear();
	void              RaiseGear();
	void              ToggleGear();
	void              ToggleNavlights();

	// WEAPON SYSTEMS:
	virtual void      CheckFriendlyFire();
	virtual void      CheckFire(bool c) { check_fire = c; }
	virtual bool      CheckFire()       const { return (check_fire || net_observer_mode) ? true : false; }
	virtual void      SelectWeapon(int n, int w);
	virtual bool      FireWeapon(int n);
	virtual bool      FirePrimary() { return FireWeapon(primary); }
	virtual bool      FireSecondary() { return FireWeapon(secondary); }
	virtual bool      FireDecoy();
	virtual void      CyclePrimary();
	virtual void      CycleSecondary();
	virtual Weapon* GetPrimary()      const;
	virtual Weapon* GetSecondary()    const;
	virtual Weapon* GetWeaponByIndex(int n);
	virtual WeaponGroup* GetPrimaryGroup()      const;
	virtual WeaponGroup* GetSecondaryGroup()    const;
	virtual Weapon* GetDecoy()        const;
	//virtual List<Shot>& GetActiveDecoys();
	virtual void      AddActiveDecoy(Drone* d);
	virtual int* GetLoadout() { return loadout; }

	List<Shot>& GetThreatList();
	void              AddThreat(Shot* s);
	void              DropThreat(Shot* s);

	bool         Update(USimObject* obj);
	const char* GetObserverName() const { return name; }

	int       GetMissileEta(int index) const;
	void      SetMissileEta(int id, int eta);

	WeaponDesign* GetPrimaryDesign()      const;
	WeaponDesign* GetSecondaryDesign()    const;

	void      SetTarget(USimObject* t, System* sub = 0, bool from_net = false);
	USimObject* GetTarget()      const { return target; }
	System* GetSubTarget()    const { return subtarget; }
	void      CycleSubTarget(int dir = 1);
	void      DropTarget();
	void      LockTarget(int  type = USimObject::SIM_SHIP, bool closest = false, bool hostile = false);
	void      LockTarget(USimObject* candidate);
	bool      IsTracking(USimObject* tgt);
	bool      GetTrigger(int i) const;
	void      SetTrigger(int i);

	UShip* GetWard()         const { return ward; }
	void              SetWard(UShip* s);

	// SHIELD SYSTEMS:
	double    InflictDamage(double damage,
		Shot* shot = 0,
		int    hit_type = 3,
		Point  hull_impact = Point(0, 0, 0));

	double    InflictSystemDamage(double damage, Shot* shot, Point impact);

	void      InflictNetDamage(double damage, Shot* shot = 0);
	void      InflictNetSystemDamage(System* system, double damage, BYTE type);
	void      SetNetSystemStatus(System* system, int status, int power, int reactor, double avail);
	void      SetIntegrity(float n) { integrity = n; }

	void      Destroy();
	int       ShieldStrength()  const;
	int       HullStrength()    const;
	int       HitBy(Shot* shot, Point& impact);
	int       CollidesWith(UPhysical& o);

	// SENSORS AND VISIBILITY:
	int       GetContactID()    const { return contact_id; }
	int       GetIFF()          const { return IFF_code; }
	void      SetIFF(int iff);
	virtual Color     MarkerColor()     const;
	static  Color     IFFColor(int iff);
	void      DoEMCON();
	double    PCS()             const;
	double    ACS()             const;
	int       NumContacts()     const;   // actual sensor contacts
	List<Contact>& ContactList();
	int       GetSensorMode()   const;
	void      SetSensorMode(int mode);
	void      LaunchProbe();
	Weapon*   GetProbeLauncher() const { return probe; }
	Drone*	  GetProbe()        const { return sensor_drone; }
	void      SetProbe(Drone* d);
	int       GetEMCON()        const { return emcon; }
	void      SetEMCON(int e, bool from_net = false);
	Contact*  FindContact(USimObject* s) const;
	bool      IsHostileTo(const USimObject* o) const;

	// GENERAL ACCESSORS:
	const char* Registry()        const { return regnum; }
	void              SetName(const char* ident) { strcpy_s(name, ident); }
	const ShipDesign* Design()          const { return design; }
	const char* Abbreviation()    const;
	const char* DesignName()      const;
	const char* DesignFileName()  const;
	static const char* ClassName(int c);
	static int        ClassForName(const char* name);
	const char* ClassName()       const;
	CLASSIFICATION    GetShipClass()    const;
	bool              IsGroundUnit()    const;
	bool              IsStarship()      const;
	bool              IsDropship()      const;
	bool              IsStatic()        const;
	bool              IsRogue()         const;
	void              SetRogue(bool r = true);
	int               GetFriendlyFire() const { return ff_count; }
	void              SetFriendlyFire(int f);
	void              IncFriendlyFire(int f = 1);
	double            Agility()         const { return agility; }
	DWORD             MissionClock()    const;
	Graphic* Cockpit()         const;
	void              ShowCockpit();
	void              HideCockpit();
	int               Value()           const;
	double            AIValue()         const;
	static int        Value(int type);

	//const Skin* GetSkin()         const { return skin; }
	//void              UseSkin(const Skin* s) { skin = s; }
	void              ShowRep();
	void              HideRep();
	void              EnableShadows(bool enable);

	int               GetRespawnCount();
	void              SetRespawnCount(int r) { respawns = r; }
	const Point& RespawnLoc()      const { return respawn_loc; }
	void              SetRespawnLoc(const Point& rl)
	{
		respawn_loc = rl;
	}

	double            WarpFactor()      const { return warp_fov; }
	void              SetWarp(double w) { warp_fov = (float)w; }

	void              MatchOrientation(const UShip& s);

	// ORDERS AND NAVIGATION:
	void                    ExecEvalFrame(double seconds);
	void                    SetLaunchPoint(Instruction* pt);
	void                    AddNavPoint(Instruction* pt, Instruction* afterPoint = 0);
	void                    DelNavPoint(Instruction* pt);
	void                    ClearFlightPlan();
	Instruction* GetNextNavPoint();
	int                     GetNavIndex(const Instruction* n);
	double                  RangeToNavPoint(const Instruction* n);
	void                    SetNavptStatus(Instruction* n, int status);
	List<Instruction>& GetFlightPlan();
	int                     FlightPlanLength();
	CombatUnit* GetCombatUnit()   const { return combat_unit; }
	Element* GetElement()      const { return element; }
	UShip* GetLeader()       const;
	int                     GetElementIndex() const;
	int                     GetOrigElementIndex() const;
	void                    SetElement(Element* e);

	Instruction* GetRadioOrders()  const;
	void                    ClearRadioOrders();
	void                    HandleRadioMessage(RadioMessage* msg);
	bool                    IsAutoNavEngaged();
	void                    SetAutoNav(bool engage = true);
	void                    CommandMode();

	void                    ClearTrack();
	void                    UpdateTrack();
	int                     TrackLength()     const { return ntrack; }
	Point                   TrackPoint(int i) const;

	// DAMAGE CONTROL AND ENGINEERING:
	List<System>& RepairQueue() { return repair_queue; }
	double                  RepairSpeed() const;
	int                     RepairTeams() const;
	void                    RepairSystem(System* sys);
	void                    IncreaseRepairPriority(int task_index);
	void                    DecreaseRepairPriority(int task_index);
	void                    ExecMaintFrame(double seconds);
	bool                    AutoRepair()    const { return auto_repair; }
	void                    EnableRepair(bool e) { auto_repair = e; }
	bool                    MasterCaution() const { return master_caution; }
	void                    ClearCaution() { master_caution = 0; }

	// SYSTEM ACCESSORS:
	List<System>& Systems() { return systems; }
	List<WeaponGroup>& Weapons() { return weapons; }
	List<Drive>& Drives() { return drives; }
	List<Computer>& Computers() { return computers; }
	List<FlightDeck>& FlightDecks() { return flight_decks; }
	List<PowerSource>& Reactors() { return reactors; }
	List<NavLight>& NavLights() { return navlights; }
	Shield* GetShield() { return shield; }
	Solid* GetShieldRep() { return (Solid*)shieldRep; }
	Sensor* GetSensor() { return sensor; }
	NavSystem* GetNavSystem() { return navsys; }
	FlightComp* GetFLCS() { return flcs; }
	Thruster* GetThruster() { return thruster; }
	Hangar* GetHangar() { return hangar; }
	LandingGear* GetGear() { return gear; }

	System* GetSystem(int sys_id);

	static int              GetControlModel() { return control_model; }
	static void             SetControlModel(int n) { control_model = n; }
	static int              GetFlightModel() { return flight_model; }
	static void             SetFlightModel(int f) { flight_model = f; }
	static int              GetLandingModel() { return landing_model; }
	static void             SetLandingModel(int f) { landing_model = f; }
	static double           GetFriendlyFireLevel() { return friendly_fire_level; }
	static void             SetFriendlyFireLevel(double f) { friendly_fire_level = f; }

protected:
	int               CheckShotIntersection(Shot* shot, Point& ipt, Point& hpt, Weapon** wep = 0);
	WeaponGroup* FindWeaponGroup(const char* name);

	char              regnum[16];
	ShipDesign* design;
	ShipKiller* killer;
	//DetailSet         detail;
	int               detail_level;
	Sim* sim;
	double            vlimit;
	double            agility;
	double            throttle;
	double            throttle_request;
	bool              augmenter;
	float             wep_mass;
	float             wep_resist;

	int               IFF_code;
	int               cmd_chain_index;
	int               ff_count;
	OP_MODE           flight_phase;

	USimObject* target;
	System* subtarget;
	UShip* ward;
	int               check_fire;
	int               primary;
	int               secondary;

	//const Skin* skin;
	Solid*				cockpit;
	Drive*				main_drive;
	QuantumDrive*		quantum_drive;
	Farcaster*			farcaster;
	Shield*				shield;
	ShieldRep*			shieldRep;
	NavSystem*			navsys;
	FlightComp*			flcs;
	Sensor*				sensor;
	LandingGear*		gear;
	Thruster*			thruster;
	Weapon*				decoy;
	Weapon*				probe;
	Drone*				sensor_drone;
	Hangar*				hangar;
	List<Shot>			decoy_list;
	List<Shot>			threat_list;

	List<System>		systems;
	List<PowerSource>	reactors;
	List<WeaponGroup>	weapons;
	List<Drive>			drives;
	List<Computer>		computers;
	List<FlightDeck>	flight_decks;
	List<NavLight>		navlights;
	List<System>		repair_queue;

	CombatUnit* combat_unit;
	Element* element;
	int               orig_elem_index;
	Instruction* radio_orders;
	Instruction* launch_point;

	Vec3              chase_vec;
	Vec3              bridge_vec;

	const char* director_info;
	BYTE              ai_mode;
	BYTE              command_ai_level;
	BYTE              flcs_mode;
	bool              net_observer_mode;

	float             pcs;  // passive sensor cross section
	float             acs;  // active  sensor cross section
	BYTE              emcon;
	BYTE              old_emcon;
	bool              invulnerable;

	DWORD             launch_time;
	DWORD             friendly_fire_time;

	UShip* carrier;
	FlightDeck* dock;
	InboundSlot* inbound;

	Director* net_control;

	Point* track;
	int               ntrack;
	DWORD             track_time;

	float             helm_heading;
	float             helm_pitch;

	float             altitude_agl;
	float             g_force;

	float             warp_fov;

	float             transition_time;
	int               transition_type;
	Point             transition_loc;
	Point             respawn_loc;
	int               respawns;

	bool              master_caution;
	bool              auto_repair;
	DWORD             last_repair_time;
	DWORD             last_eval_time;
	DWORD             last_beam_time;
	DWORD             last_bolt_time;

	int               missile_id[4];
	BYTE              missile_eta[4];
	bool              trigger[4];
	int* loadout;

	int               contact_id;

	static int        control_model;
	static int        flight_model;
	static int        landing_model;
	static double     friendly_fire_level;
};
