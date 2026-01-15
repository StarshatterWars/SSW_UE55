/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         MissionElement.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Navigation Point class implementation
*/

#pragma once

#include "CoreMinimal.h"
#include "Types.h"
#include "Intel.h"
#include "RLoc.h"
#include "Universe.h"
//#include "Scene.h"
//#include "Skin.h"
#include "Physical.h"
#include "Geometry.h"
#include "Color.h"
#include "List.h"
#include "Text.h"
#include "SSWGameInstance.h"

// +--------------------------------------------------------------------+

class Mission;
class MissionElement;
class MissionLoad;
class MissionEvent;
class MissionShip;

class CombatGroup;
class CombatUnit;

class Ship;
class System;
class Element;
class ShipDesign;
class WeaponDesign;
class AStarSystem;
class Instruction;

class Term;
class TermArray;
class TermStruct;

// +--------------------------------------------------------------------+
/**
 * 
 */
class STARSHATTERWARS_API MissionElement
{
public:

	friend class Mission;

	static const char* TYPENAME() { return "MissionElement"; }

	MissionElement();
	~MissionElement();

	int operator == (const MissionElement& r) const { return id == r.id; }

	int               Identity()     const { return id; }
	const Text& Name()         const { return name; }
	Text              Abbreviation() const;
	const Text& Carrier()      const { return carrier; }
	const Text& Commander()    const { return commander; }
	const Text& Squadron()     const { return squadron; }
	const Text& Path()         const { return path; }
	int               ElementID()    const { return elem_id; }
	const ShipDesign* GetDesign()    const { return design; }
	//const Skin* GetSkin()      const { return skin; }
	int               Count()        const { return count; }
	int               MaintCount()   const { return maint_count; }
	int               DeadCount()    const { return dead_count; }
	int               GetIFF()       const { return IFF_code; }
	int               IntelLevel()   const { return intel; }
	int               MissionRole()  const { return mission_role; }
	int               Player()       const { return player; }
	Text              RoleName()     const;
	Color             MarkerColor()  const;
	bool              IsStarship()   const;
	bool              IsDropship()   const;
	bool              IsStatic()     const;
	bool              IsGroundUnit() const;
	bool              IsSquadron()   const;
	bool              IsCarrier()    const;
	bool              IsAlert()      const { return alert; }
	bool              IsPlayable()   const { return playable; }
	bool              IsRogue()      const { return rogue; }
	bool              IsInvulnerable() const { return invulnerable; }
	int               RespawnCount() const { return respawns; }
	int               HoldTime()     const { return hold_time; }
	int               CommandAI()    const { return command_ai; }
	int               ZoneLock()     const { return zone_lock; }

	const Text& Region()       const { return rgn_name; }
	Point             Location()     const;
	RLoc& GetRLoc() { return rloc; }
	double            Heading()      const { return heading; }

	Text              GetShipName(int n) const;
	Text              GetRegistry(int n) const;

	List<Instruction>& Objectives() { return objectives; }
	List<Text>& Instructions() { return instructions; }
	List<Instruction>& NavList() { return navlist; }
	List<MissionLoad>& Loadouts() { return loadouts; }
	List<MissionShip>& Ships() { return ships; }

	void              SetName(const char* n) { name = n; }
	void              SetCarrier(const char* c) { carrier = c; }
	void              SetCommander(const char* c) { commander = c; }
	void              SetSquadron(const char* s) { squadron = s; }
	void              SetPath(const char* p) { path = p; }
	void              SetElementID(int eid) { elem_id = eid; }
	void              SetDesign(const ShipDesign* d) { design = d; }
	//void              SetSkin(const Skin* s) { skin = s; }
	void              SetCount(int n) { count = n; }
	void              SetMaintCount(int n) { maint_count = n; }
	void              SetDeadCount(int n) { dead_count = n; }
	void              SetIFF(int iff) { IFF_code = iff; }
	void              SetIntelLevel(int i) { intel = i; }
	void              SetMissionRole(int r) { mission_role = r; }
	void              SetPlayer(int p) { player = p; }
	void              SetPlayable(bool p) { playable = p; }
	void              SetRogue(bool r) { rogue = r; }
	void              SetInvulnerable(bool n) { invulnerable = n; }
	void              SetAlert(bool a) { alert = a; }
	void              SetCommandAI(int a) { command_ai = a; }
	void              SetRegion(const char* rgn) { rgn_name = rgn; }
	void              SetLocation(const Point& p);
	void              SetRLoc(const RLoc& r);
	void              SetHeading(double h) { heading = h; }
	void              SetRespawnCount(int r) { respawns = r; }
	void              SetHoldTime(int t) { hold_time = t; }
	void              SetZoneLock(int z) { zone_lock = z; }

	void              AddNavPoint(Instruction* pt, Instruction* afterPoint = 0);
	void              DelNavPoint(Instruction* pt);
	void              ClearFlightPlan();
	int               GetNavIndex(const Instruction* n);

	void              AddObjective(Instruction* obj) { objectives.append(obj); }
	void              AddInstruction(const char* i) { instructions.append(new Text(i)); }

	CombatGroup* GetCombatGroup() { return combat_group; }
	void              SetCombatGroup(CombatGroup* g) { combat_group = g; }
	CombatUnit* GetCombatUnit() { return combat_unit; }
	void              SetCombatUnit(CombatUnit* u) { combat_unit = u; }

protected:
	int               id;
	Text              name;
	Text              carrier;
	Text              commander;
	Text              squadron;
	Text              path;
	int               elem_id;
	const ShipDesign* design;
	//const Skin* skin;
	int               count;
	int               maint_count;
	int               dead_count;
	int               IFF_code;
	int               mission_role;
	int               intel;
	int               respawns;
	int               hold_time;
	int               zone_lock;
	int               player;
	int               command_ai;
	bool              alert;
	bool              playable;
	bool              rogue;
	bool              invulnerable;

	Text              rgn_name;
	RLoc              rloc;
	double            heading;

	CombatGroup* combat_group;
	CombatUnit* combat_unit;

	List<Instruction> objectives;
	List<Text>        instructions;
	List<Instruction> navlist;
	List<MissionLoad> loadouts;
	List<MissionShip> ships;
};
