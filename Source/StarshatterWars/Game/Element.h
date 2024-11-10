/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         Element.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Package Element (e.g. Flight) class
*/

#pragma once

#include "CoreMinimal.h"
#include "../Foundation/Types.h"
#include "../Foundation/Geometry.h"
#include "SimObject.h"
#include "../Foundation/List.h"
#include "../Foundation/Text.h"

// +--------------------------------------------------------------------+

class UShip;
class Instruction;
class RadioMessage;
class Element;
class CombatGroup;
class CombatUnit;

// +--------------------------------------------------------------------+
/**
 * 
 */
class STARSHATTERWARS_API Element : SimObserver
{
public:
public:
	// CONSTRUCTORS:
	Element();
	Element(const char* call_sign, int iff, int type = 0 /*PATROL*/);

	int operator == (const Element& e) const { return id == e.id; }

	// GENERAL ACCESSORS:
	int               Identity()        const { return id; }
	int               Type()            const { return type; }
	const Text& Name()            const { return name; }
	void              SetName(const char* s) { name = s; }
	virtual int       GetIFF()          const { return iff; }
	int               Player()          const { return player; }
	void              SetPlayer(int p) { player = p; }
	DWORD             GetLaunchTime()   const { return launch_time; }
	void              SetLaunchTime(DWORD t);
	int               IntelLevel()      const { return intel; }
	void              SetIntelLevel(int i) { intel = i; }

	// ELEMENT COMPONENTS:
	int					NumShips() const { return ships.size(); }
	int					AddShip(UShip*, int index = -1);
	void				DelShip(UShip*);
	UShip*				GetShip(int index);
	int                 GetShipClass();
	int					FindIndex(const UShip* s);
	bool				Contains(const UShip* s);
	bool                IsActive()        const;
	bool                IsFinished()      const;
	bool                IsNetObserver()   const;
	bool                IsSquadron()      const;
	bool                IsStatic()        const;
	bool				IsHostileTo(const UShip* s)	const;
	bool                IsHostileTo(int iff_code)	const;
	bool				IsObjectiveTargetOf(const UShip* s)  const;
	bool                IsRogue()         const { return rogue; }
	bool                IsPlayable()      const { return playable; }
	int*			    Loadout() { return load; }

	void               SetRogue(bool r) { rogue = r; }
	void               SetPlayable(bool p) { playable = p; }
	void               SetLoadout(int* l);
	void			   SetIFF(int iff);

	void			   ExecFrame(double seconds);
	bool			   Update(USimObject* obj);
	const char*		   GetObserverName() const;

	// OBJECTIVES:
	void               ClearObjectives();
	void               AddObjective(Instruction* obj);
	Instruction*	   GetObjective(int index);
	Instruction*	   GetTargetObjective();
	int                NumObjectives()   const { return objectives.size(); }

	void               ClearInstructions();
	void               AddInstruction(const char* instr);
	Text               GetInstruction(int index);
	int                NumInstructions()   const { return instructions.size(); }

	// ORDERS AND NAVIGATION:
	double             GetHoldTime();
	void               SetHoldTime(double t);
	bool               GetZoneLock();
	void               SetZoneLock(bool z);
	void               AddNavPoint(Instruction* pt, Instruction* afterPoint = 0, bool send = true);
	void               DelNavPoint(Instruction* pt, bool send = true);
	void               ClearFlightPlan(bool send = true);
	Instruction*	   GetNextNavPoint();
	int                GetNavIndex(const Instruction* n);
	List<Instruction>& GetFlightPlan();
	int                FlightPlanLength();
	//virtual void      HandleRadioMessage(RadioMessage* msg);

	// CHAIN OF COMMAND:
	Element*			GetCommander()             const { return commander; }
	void                SetCommander(Element* e) { commander = e; }
	Element*			GetAssignment()            const { return assignment; }
	void				SetAssignment(Element* e) { assignment = e; }
	void				ResumeAssignment();
	bool				CanCommand(Element* e);
	UShip*				GetCarrier()               const { return carrier; }
	void				SetCarrier(UShip* c) { carrier = c; }
	int					GetCommandAILevel()        const { return command_ai; }
	void				SetCommandAILevel(int n) { command_ai = n; }
	const Text&			GetSquadron()              const { return squadron; }
	void				SetSquadron(const char* s) { squadron = s; }

	// DYNAMIC CAMPAIGN:
	CombatGroup*		GetCombatGroup() { return combat_group; }
	void				SetCombatGroup(CombatGroup* g) { combat_group = g; }
	CombatUnit*			GetCombatUnit() { return combat_unit; }
	void				SetCombatUnit(CombatUnit* u) { combat_unit = u; }

	// SQUADRON STUFF:
	int					GetCount()                 const { return count; }
	void				SetCount(int n) { count = n; }

protected:
	int               id;
	int               iff;
	int               type;
	int               player;
	int               command_ai;
	int               respawns;
	int               intel;
	Text              name;

	// squadron elements only:
	int               count;

	List<UShip>        ships;
	List<Text>        ship_names;
	List<Text>        instructions;
	List<Instruction> objectives;
	List<Instruction> flight_plan;

	Element* commander;
	Element* assignment;
	UShip* carrier;
	Text              squadron;

	CombatGroup* combat_group;
	CombatUnit* combat_unit;
	DWORD             launch_time;
	double            hold_time;

	bool              rogue;
	bool              playable;
	bool              zone_lock;
	int               load[16];

	int id_key;
};
