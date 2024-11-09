/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         Mission.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Simulation Universe and Region classes
*/

#pragma once

#include "../Foundation/Types.h"
//#include "Intel.h"
#include "../Foundation/RLoc.h"
#include "../Space/Universe.h"
//#include "Scene.h"
//#include "Skin.h"
//#include "Physical.h"
#include "../Foundation/Geometry.h"
#include "../Foundation/List.h"
#include "../Foundation/Text.h"
#include "../System/SSWGameInstance.h"


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
class STARSHATTERWARS_API Mission
{
	static const char* TYPENAME() { return "Mission"; }

public:
	
	enum TYPE
	{
		PATROL,
		SWEEP,
		INTERCEPT,
		AIR_PATROL,
		AIR_SWEEP,
		AIR_INTERCEPT,
		STRIKE,     // ground attack
		ASSAULT,    // starship attack
		DEFEND,
		ESCORT,
		ESCORT_FREIGHT,
		ESCORT_SHUTTLE,
		ESCORT_STRIKE,
		INTEL,
		SCOUT,
		RECON,
		BLOCKADE,
		FLEET,
		BOMBARDMENT,
		FLIGHT_OPS,
		TRANSPORT,
		CARGO,
		TRAINING,
		OTHER
	};
	
	Mission();

	Mission(int id, const char* filename = 0, const char* path = 0);

	int operator== (const Mission& m)   const { return id == m.id; }

	void            Validate();
	bool            Load(const char* fn = 0, const char* fp = 0);
	bool            Save();
	bool            ParseMission(const char* buffer);
	void            SetPlayer(MissionElement* player_element);
	MissionElement* GetPlayer();

	// accessors/mutators:
	int                  Identity()      const { return id; }
	const char* FileName()      const { return filename; }
	const char* Name()          const { return name; }
	const char* Description()   const { return desc; }
	const char* Situation()     const { return sitrep; }
	const char* Objective()     const { return objective; }
	const char* Subtitles()     const;
	int                  Start()         const { return start; }
	double               Stardate()      const { return stardate; }
	int                  Type()          const { return type; }
	const char* TypeName()      const { return RoleName(type); }
	int                  Team()          const { return team; }
	bool                 IsOK()          const { return ok; }
	bool                 IsActive()      const { return active; }
	bool                 IsComplete()    const { return complete; }

	AStarSystem* GetStarSystem() const { return star_system; }
	List<AStarSystem>& GetSystemList() { return system_list; }
	const char* GetRegion()     const { return region; }

	List<MissionElement>& GetElements() { return elements; }
	MissionElement* FindElement(const char* elem_name);
	void            AddElement(MissionElement* elem);

	List<MissionEvent>& GetEvents() { return events; }
	MissionEvent* FindEvent(int event_type) const;
	virtual void         AddEvent(MissionEvent* event);

	MissionElement* GetTarget()     const { return target; }
	MissionElement* GetWard()       const { return ward; }

	void                 SetName(const char* n) { name = n; }
	void                 SetDescription(const char* d) { desc = d; }
	void                 SetSituation(const char* sit) { sitrep = sit; }
	void                 SetObjective(const char* obj) { objective = obj; }
	void                 SetStart(int s) { start = s; }
	void                 SetType(int t) { type = t; }
	void                 SetTeam(int iff) { team = iff; }
	void                 SetStarSystem(AStarSystem* s);
	void                 SetRegion(const char* rgn) { region = rgn; }
	void                 SetOK(bool a) { ok = a; }
	void                 SetActive(bool a) { active = a; }
	void                 SetComplete(bool c) { complete = c; }
	void                 SetTarget(MissionElement* t) { target = t; }
	void                 SetWard(MissionElement* w) { ward = w; }

	void                 ClearSystemList();

	void                 IncreaseElemPriority(int index);
	void                 DecreaseElemPriority(int index);
	void                 IncreaseEventPriority(int index);
	void                 DecreaseEventPriority(int index);

	static const char* RoleName(int role);
	static int           TypeFromName(const char* n);

	Text                 ErrorMessage() const { return errmsg; }
	void                 AddError(Text err);

	Text                 Serialize(const char* player_elem = 0, int player_index = 0);

protected:
	MissionElement* ParseElement(TermStruct* val);
	MissionEvent*	ParseEvent(TermStruct* val);
	MissionShip*	ParseShip(TermStruct* val, MissionElement* element);
	Instruction*	ParseInstruction(TermStruct* val, MissionElement* element);
	void			ParseLoadout(TermStruct* val, MissionElement* element);
	RLoc*			ParseRLoc(TermStruct* val);

	int                  id;
	char                 filename[64];
	char                 path[64];
	Text                 region;
	Text                 name;
	Text                 desc;
	int                  type;
	int                  team;
	int                  start;
	double               stardate;
	bool                 ok;
	bool                 active;
	bool                 complete;
	bool                 degrees;
	Text                 objective;
	Text                 sitrep;
	Text                 errmsg;
	Text                 subtitles;
	AStarSystem* star_system;
	List<AStarSystem>     system_list;

	List<MissionElement> elements;
	List<MissionEvent>   events;

	MissionElement* target;
	MissionElement* ward;
	MissionElement* current;
};
