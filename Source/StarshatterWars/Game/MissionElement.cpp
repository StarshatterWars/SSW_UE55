/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         MissionElement.cpp
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Navigation Point class implementation
*/


#include "MissionElement.h"
#include "Mission.h"
#include "MissionShip.h"
#include "MissionLoad.h"
#include "ShipDesign.h"
#include "Ship.h"
#include "Instruction.h"

// +====================================================================+

static int elem_idkey = 1;

MissionElement::MissionElement()
{
	id = elem_idkey++;
	elem_id = 0;
	design = 0;
	//skin = 0;
	count = 1;
	maint_count = 0;
	dead_count = 0;
	IFF_code = 0;
	player = 0;
	alert = false;
	playable = false;
	rogue = false;
	invulnerable = false;
	respawns = 0;
	hold_time = 0;
	zone_lock = 0;
	heading = 0;
	mission_role = Mission::OTHER;
	intel = INTEL_TYPE::SECRET;
	command_ai = 1;
	combat_group = 0;
	combat_unit = 0;
}

MissionElement::~MissionElement()
{
	ships.destroy();
	objectives.destroy();
	instructions.destroy();
	navlist.destroy();
	loadouts.destroy();
}

Text
MissionElement::Abbreviation() const
{
	if (design)
		return design->abrv;

	return "UNK";
}

Text
MissionElement::GetShipName(int index) const
{
	if (index < 0 || index >= ships.size()) {
		if (count > 1) {
			char sname[256];
			sprintf_s(sname, "%s %d", (const char*)name, index + 1);
			return sname;
		}
		else {
			return name;
		}
	}

	return ships.at(index)->Name();
}

Text
MissionElement::GetRegistry(int index) const
{
	if (index < 0 || index >= ships.size()) {
		return Text();
	}

	return ships.at(index)->RegNum();
}

Text
MissionElement::RoleName() const
{
	return Mission::RoleName(mission_role);
}

Color
MissionElement::MarkerColor() const
{
	return UShip::IFFColor(IFF_code);
}

bool
MissionElement::IsStatic() const
{
	int design_type = 0;
	if (GetDesign())
		design_type = GetDesign()->type;

	return design_type >= (int) CLASSIFICATION::STATION;
}

bool
MissionElement::IsGroundUnit() const
{
	int design_type = 0;
	if (GetDesign())
		design_type = GetDesign()->type;

	return (design_type & (int)CLASSIFICATION::GROUND_UNITS) ? true : false;
}

bool
MissionElement::IsStarship() const
{
	int design_type = 0;
	if (GetDesign())
		design_type = GetDesign()->type;

	return (design_type & (int) CLASSIFICATION::STARSHIPS) ? true : false;
}

bool
MissionElement::IsDropship() const
{
	int design_type = 0;
	if (GetDesign())
		design_type = GetDesign()->type;

	return (design_type & (int)CLASSIFICATION::DROPSHIPS) ? true : false;
}

bool
MissionElement::IsCarrier() const
{
	const ShipDesign* dsgn = GetDesign();
	if (dsgn && dsgn->flight_decks.size() > 0)
		return true;

	return false;
}

bool
MissionElement::IsSquadron() const
{
	if (carrier.length() > 0)
		return true;

	return false;
}

// +--------------------------------------------------------------------+

Point
MissionElement::Location() const
{
	MissionElement* pThis = (MissionElement*)this;
	return pThis->rloc.Location();
}

void
MissionElement::SetLocation(const Point& l)
{
	rloc.SetBaseLocation(l);
	rloc.SetReferenceLoc(0);
	rloc.SetDistance(0);
}

void
MissionElement::SetRLoc(const RLoc& r)
{
	rloc = r;
}

// +----------------------------------------------------------------------+

void
MissionElement::AddNavPoint(Instruction* pt, Instruction* afterPoint)
{
	if (pt && !navlist.contains(pt)) {
		if (afterPoint) {
			int index = navlist.index(afterPoint);

			if (index > -1)
				navlist.insert(pt, index + 1);
			else
				navlist.append(pt);
		}

		else {
			navlist.append(pt);
		}
	}
}

void
MissionElement::DelNavPoint(Instruction* pt)
{
	if (pt)
		delete navlist.remove(pt);
}

void
MissionElement::ClearFlightPlan()
{
	navlist.destroy();
}

// +----------------------------------------------------------------------+

int
MissionElement::GetNavIndex(const Instruction* n)
{
	int index = 0;

	if (navlist.size() > 0) {
		ListIter<Instruction> navpt = navlist;
		while (++navpt) {
			index++;
			if (navpt.value() == n)
				return index;
		}
	}

	return 0;
}

// +====================================================================+