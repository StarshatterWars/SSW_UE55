/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         CombatGroup.cpp
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	One group in a military conflict
*/


#include "CombatGroup.h"
#include "CombatGroup.h"
#include "CombatUnit.h"
#include "CombatZone.h"
#include "Combatant.h"
#include "CombatAssignment.h"
#include "Campaign.h"
//#include "ShipDesign.h"
#include "Ship.h"

#include "../System/Game.h"
#include "../Foundation/DataLoader.h"
#include "../Foundation/ParseUtil.h"

// +----------------------------------------------------------------------+

CombatGroup::CombatGroup(int t, int n, const char* s, int iff_code, int e, CombatGroup* p)
	: type(t), id(n), name(s), iff(iff_code), enemy_intel(e),
	parent(p), value(0), plan_value(0), unit_index(0), combatant(0),
	expanded(false), sorties(0), kills(0), points(0),
	current_zone(0), assigned_zone(0), zone_lock(false)
{
	if (parent)
		parent->AddComponent(this);
}

CombatGroup::~CombatGroup()
{
	assignments.destroy();
	components.destroy();
	units.destroy();
}

// +--------------------------------------------------------------------+

CombatGroup* CombatGroup::LoadOrderOfBattle(const char* fname, int iff, Combatant* combatant)
{
	return nullptr;
}

void
CombatGroup::AddComponent(CombatGroup* g)
{
	if (g) {
		g->parent = this;
		components.append(g);
	}
}

// +--------------------------------------------------------------------+

bool
CombatGroup::IsAssignable() const
{
	switch (type) {
	case (int8) ECOMBATGROUP_TYPE::CARRIER_GROUP:
	case (int8) ECOMBATGROUP_TYPE::BATTLE_GROUP:
	case (int8) ECOMBATGROUP_TYPE::DESTROYER_SQUADRON:
	case (int8) ECOMBATGROUP_TYPE::ATTACK_SQUADRON:
	case (int8) ECOMBATGROUP_TYPE::FIGHTER_SQUADRON:
	case (int8) ECOMBATGROUP_TYPE::INTERCEPT_SQUADRON:
	case (int8) ECOMBATGROUP_TYPE::LCA_SQUADRON:
		return ((CombatGroup*)this)->CalcValue() > 0;
	}

	return false;
}

bool
CombatGroup::IsTargetable() const
{
	// neutral / non-combatants are not *strategic* targets
	// for any combatant:
	if (iff < 1 || iff >= 100)
		return false;

	// civilian / non-combatant are not strategic targets:
	if (type == (int) ECOMBATGROUP_TYPE::PASSENGER ||
		type == (int) ECOMBATGROUP_TYPE::PRIVATE ||
		type == (int) ECOMBATGROUP_TYPE::MEDICAL ||
		type == (int) ECOMBATGROUP_TYPE::HABITAT)
		return false;

	// must have units of our own to be targetable:
	if (units.size() < 1)
		return false;

	return ((CombatGroup*)this)->CalcValue() > 0;
}

bool
CombatGroup::IsDefensible() const
{
	if (type >= SUPPORT)
		return ((CombatGroup*)this)->CalcValue() > 0;

	return false;
}

bool
CombatGroup::IsStrikeTarget() const
{
	if (type < (int) ECOMBATGROUP_TYPE::BATTALION ||
		type == (int) ECOMBATGROUP_TYPE::MINEFIELD ||   // assault, not strike
		type == (int) ECOMBATGROUP_TYPE::PASSENGER ||
		type == (int) ECOMBATGROUP_TYPE::PRIVATE ||
		type == (int) ECOMBATGROUP_TYPE::MEDICAL ||
		type == (int) ECOMBATGROUP_TYPE::HABITAT)
		return false;

	return ((CombatGroup*)this)->CalcValue() > 0;
}

// +--------------------------------------------------------------------+

bool
CombatGroup::IsMovable() const
{
	switch (type) {
	case (int)ECOMBATGROUP_TYPE::CARRIER_GROUP:
	case (int)ECOMBATGROUP_TYPE::BATTLE_GROUP:
	case (int)ECOMBATGROUP_TYPE::DESTROYER_SQUADRON:
	case (int)ECOMBATGROUP_TYPE::ATTACK_SQUADRON:
	case (int)ECOMBATGROUP_TYPE::FIGHTER_SQUADRON:
	case (int)ECOMBATGROUP_TYPE::INTERCEPT_SQUADRON:
	case (int)ECOMBATGROUP_TYPE::LCA_SQUADRON:
	case (int)ECOMBATGROUP_TYPE::COURIER:
	case (int)ECOMBATGROUP_TYPE::MEDICAL:
	case (int)ECOMBATGROUP_TYPE::SUPPLY:
	case (int)ECOMBATGROUP_TYPE::REPAIR:
	case (int)ECOMBATGROUP_TYPE::FREIGHT:
	case (int)ECOMBATGROUP_TYPE::PASSENGER:
	case (int)ECOMBATGROUP_TYPE::PRIVATE:
		return true;
	}

	return false;
}

// +--------------------------------------------------------------------+

bool
CombatGroup::IsFighterGroup() const
{
	switch (type) {
	case (int)ECOMBATGROUP_TYPE::WING:
	case (int)ECOMBATGROUP_TYPE::INTERCEPT_SQUADRON:
	case (int)ECOMBATGROUP_TYPE::FIGHTER_SQUADRON:
	case (int)ECOMBATGROUP_TYPE::ATTACK_SQUADRON:
		return true;
	}

	return false;
}

bool
CombatGroup::IsStarshipGroup() const
{
	switch (type) {
	case (int)ECOMBATGROUP_TYPE::DESTROYER_SQUADRON:
	case (int)ECOMBATGROUP_TYPE::BATTLE_GROUP:
	case (int)ECOMBATGROUP_TYPE::CARRIER_GROUP:
		return true;
	}

	return false;
}

// +--------------------------------------------------------------------+

bool
CombatGroup::IsReserve() const
{
	if (enemy_intel <= (int)Intel::RESERVE)
		return true;

	if (parent)
		return parent->IsReserve();

	return false;
}

// +--------------------------------------------------------------------+

const int*
CombatGroup::PreferredAttacker(int type)
{
	static int p[8];

	ZeroMemory(p, sizeof(p));

	switch (type) {
		//case FLEET:
	case (int)ECOMBATGROUP_TYPE::DESTROYER_SQUADRON:   
		p[0] = (int)ECOMBATGROUP_TYPE::DESTROYER_SQUADRON;
		p[1] = (int)ECOMBATGROUP_TYPE::BATTLE_GROUP;
		p[2] = (int)ECOMBATGROUP_TYPE::CARRIER_GROUP;
		p[3] = (int)ECOMBATGROUP_TYPE::ATTACK_SQUADRON;
		break;

	case (int)ECOMBATGROUP_TYPE::BATTLE_GROUP:         
		p[0] = BATTLE_GROUP;
		p[1] = (int)ECOMBATGROUP_TYPE::DESTROYER_SQUADRON;
		p[2] = (int)ECOMBATGROUP_TYPE::CARRIER_GROUP;
		p[3] = (int)ECOMBATGROUP_TYPE::ATTACK_SQUADRON;
		break;

	case (int)ECOMBATGROUP_TYPE::CARRIER_GROUP:  
		p[0] = (int)ECOMBATGROUP_TYPE::ATTACK_SQUADRON;
		p[1] = (int)ECOMBATGROUP_TYPE::BATTLE_GROUP;
		p[2] = (int)ECOMBATGROUP_TYPE::DESTROYER_SQUADRON;
		p[3] = (int)ECOMBATGROUP_TYPE::CARRIER_GROUP;
		break;

		//case WING:
	case (int)ECOMBATGROUP_TYPE::LCA_SQUADRON:
	case (int)ECOMBATGROUP_TYPE::ATTACK_SQUADRON:
	case (int)ECOMBATGROUP_TYPE::INTERCEPT_SQUADRON:
	case (int)ECOMBATGROUP_TYPE::FIGHTER_SQUADRON:   
		p[0] = (int)ECOMBATGROUP_TYPE::INTERCEPT_SQUADRON;
		p[1] = (int)ECOMBATGROUP_TYPE::FIGHTER_SQUADRON;
		break;

		//case BATTALION:
	case (int)ECOMBATGROUP_TYPE::STATION:       
		p[0] = (int)ECOMBATGROUP_TYPE::BATTLE_GROUP;
		p[1] = (int)ECOMBATGROUP_TYPE::CARRIER_GROUP;
		break;

	case (int)ECOMBATGROUP_TYPE::STARBASE:
	case (int)ECOMBATGROUP_TYPE::BATTERY:
	case (int)ECOMBATGROUP_TYPE::MISSILE:    
		p[0] = (int)ECOMBATGROUP_TYPE::ATTACK_SQUADRON;
		p[1] = (int)ECOMBATGROUP_TYPE::FIGHTER_SQUADRON;
		break;

		//case C3I:
	case (int)ECOMBATGROUP_TYPE::MINEFIELD:
	case (int)ECOMBATGROUP_TYPE::COMM_RELAY:
	case (int)ECOMBATGROUP_TYPE::EARLY_WARNING:
	case (int)ECOMBATGROUP_TYPE::FWD_CONTROL_CTR:
	case (int)ECOMBATGROUP_TYPE::ECM:       
		p[0] = (int)ECOMBATGROUP_TYPE::ATTACK_SQUADRON;
		p[1] = (int)ECOMBATGROUP_TYPE::FIGHTER_SQUADRON;
		p[2] = (int)ECOMBATGROUP_TYPE::DESTROYER_SQUADRON;
		break;

		//case SUPPORT:
	case (int)ECOMBATGROUP_TYPE::COURIER:
	case (int)ECOMBATGROUP_TYPE::MEDICAL:
	case (int)ECOMBATGROUP_TYPE::SUPPLY:
	case (int)ECOMBATGROUP_TYPE::REPAIR:   
		p[0] = (int)ECOMBATGROUP_TYPE::DESTROYER_SQUADRON;
		p[1] = (int)ECOMBATGROUP_TYPE::BATTLE_GROUP;
		p[2] = (int)ECOMBATGROUP_TYPE::ATTACK_SQUADRON;
		break;

		//case CIVILIAN:

		//case WAR_PRODuCTION:
	case (int)ECOMBATGROUP_TYPE::FACTORY:
	case (int)ECOMBATGROUP_TYPE::REFINERY:
	case (int)ECOMBATGROUP_TYPE::RESOURCE:   
		p[0] = (int)ECOMBATGROUP_TYPE::ATTACK_SQUADRON;
		p[1] = (int)ECOMBATGROUP_TYPE::FIGHTER_SQUADRON;
		break;

		//case INFRASTRUCTURE:
	case (int)ECOMBATGROUP_TYPE::TRANSPORT:
	case (int)ECOMBATGROUP_TYPE::NETWORK:
	case (int)ECOMBATGROUP_TYPE::HABITAT:
	case (int)ECOMBATGROUP_TYPE::STORAGE:   
		p[0] = (int)ECOMBATGROUP_TYPE::ATTACK_SQUADRON;
		p[1] = (int)ECOMBATGROUP_TYPE::FIGHTER_SQUADRON;
		break;

		//case NON_COM:
	case (int)ECOMBATGROUP_TYPE::FREIGHT:
	case (int)ECOMBATGROUP_TYPE::PASSENGER:
	case (int)ECOMBATGROUP_TYPE::PRIVATE:  
		p[0] = (int)ECOMBATGROUP_TYPE::DESTROYER_SQUADRON;
		p[1] = (int)ECOMBATGROUP_TYPE::ATTACK_SQUADRON;
		break;
	}

	return p;
}

// +--------------------------------------------------------------------+

const int*
CombatGroup::PreferredDefender(int type)
{
	static int p[8];

	ZeroMemory(p, sizeof(p));

	switch (type) {
		//case FLEET:
	case (int)ECOMBATGROUP_TYPE::CARRIER_GROUP:
	case (int)ECOMBATGROUP_TYPE::BATTLE_GROUP:
	case (int)ECOMBATGROUP_TYPE::DESTROYER_SQUADRON:

		//case WING:
	case (int)ECOMBATGROUP_TYPE::LCA_SQUADRON:
	case (int)ECOMBATGROUP_TYPE::ATTACK_SQUADRON:
	case (int)ECOMBATGROUP_TYPE::INTERCEPT_SQUADRON:
	case (int)ECOMBATGROUP_TYPE::FIGHTER_SQUADRON:     break;

		//case BATTALION:
	case (int)ECOMBATGROUP_TYPE::STATION:   
		p[0] = (int)ECOMBATGROUP_TYPE::BATTLE_GROUP;
		p[1] = (int)ECOMBATGROUP_TYPE::CARRIER_GROUP;
		p[2] = (int)ECOMBATGROUP_TYPE::DESTROYER_SQUADRON;
		break;
	case (int)ECOMBATGROUP_TYPE::STARBASE:
	case (int)ECOMBATGROUP_TYPE::MINEFIELD:
	case (int)ECOMBATGROUP_TYPE::BATTERY:
	case (int)ECOMBATGROUP_TYPE::MISSILE:     
		p[0] = (int)ECOMBATGROUP_TYPE::FIGHTER_SQUADRON;
		p[1] = (int)ECOMBATGROUP_TYPE::INTERCEPT_SQUADRON;
		break;

		//case C3I:
	case (int)ECOMBATGROUP_TYPE::COMM_RELAY:
	case (int)ECOMBATGROUP_TYPE::EARLY_WARNING:
	case (int)ECOMBATGROUP_TYPE::FWD_CONTROL_CTR:
	case (int)ECOMBATGROUP_TYPE::ECM:   
		p[0] = (int)ECOMBATGROUP_TYPE::FIGHTER_SQUADRON;
		p[1] = (int)ECOMBATGROUP_TYPE::INTERCEPT_SQUADRON;
		break;

		//case SUPPORT:
	case (int)ECOMBATGROUP_TYPE::COURIER:
	case (int)ECOMBATGROUP_TYPE::MEDICAL:
	case (int)ECOMBATGROUP_TYPE::SUPPLY:
	case (int)ECOMBATGROUP_TYPE::REPAIR:   
		p[0] = (int)ECOMBATGROUP_TYPE::DESTROYER_SQUADRON;
		p[1] = (int)ECOMBATGROUP_TYPE::BATTLE_GROUP;
		p[2] = (int)ECOMBATGROUP_TYPE::ATTACK_SQUADRON;
		break;

		//case CIVILIAN:

		//case WAR_PRODuCTION:
	case (int)ECOMBATGROUP_TYPE::FACTORY:
	case (int)ECOMBATGROUP_TYPE::REFINERY:
	case (int)ECOMBATGROUP_TYPE::RESOURCE:   
		p[0] = (int)ECOMBATGROUP_TYPE::FIGHTER_SQUADRON;
		p[1] = (int)ECOMBATGROUP_TYPE::INTERCEPT_SQUADRON;
		break;

		//case INFRASTRUCTURE:
	case (int)ECOMBATGROUP_TYPE::TRANSPORT:
	case (int)ECOMBATGROUP_TYPE::NETWORK:
	case (int)ECOMBATGROUP_TYPE::HABITAT:
	case (int)ECOMBATGROUP_TYPE::STORAGE:    
		p[0] = (int)ECOMBATGROUP_TYPE::FIGHTER_SQUADRON;
		p[1] = (int)ECOMBATGROUP_TYPE::INTERCEPT_SQUADRON;
		break;

		//case NON_COM:
	case (int)ECOMBATGROUP_TYPE::FREIGHT:
	case (int)ECOMBATGROUP_TYPE::PASSENGER:
	case (int)ECOMBATGROUP_TYPE::PRIVATE:    
		p[0] = (int)ECOMBATGROUP_TYPE::DESTROYER_SQUADRON;
		p[1] = (int)ECOMBATGROUP_TYPE::BATTLE_GROUP;
		break;
	}

	return p;
}

// +--------------------------------------------------------------------+

CombatGroup*
CombatGroup::FindGroup(int t, int n)
{
	CombatGroup* result = 0;

	if (type == t && (n < 0 || id == n))
		result = this;

	ListIter<CombatGroup> group = components;
	while (!result && ++group) {
		result = group->FindGroup(t, n);
	}

	return result;
}

// +--------------------------------------------------------------------+

CombatGroup*
CombatGroup::Clone(bool deep)
{
	CombatGroup* clone = new CombatGroup(type, id, name, iff, enemy_intel);

	clone->combatant = combatant;
	clone->region = region;
	clone->location = location;
	clone->value = value;
	clone->expanded = expanded;

	for (int i = 0; i < units.size(); i++) {
		CombatUnit* u = new CombatUnit(*units[i]);
		u->SetCombatGroup(clone);
		clone->units.append(u);
	}

	if (deep) {
		for (int i = 0; i < components.size(); i++) {
			CombatGroup* g = components[i]->Clone(deep);
			clone->AddComponent(g);

			if (g->Type() == (int)ECOMBATGROUP_TYPE::FIGHTER_SQUADRON ||
				g->Type() == (int)ECOMBATGROUP_TYPE::INTERCEPT_SQUADRON ||
				g->Type() == (int)ECOMBATGROUP_TYPE::ATTACK_SQUADRON ||
				g->Type() == (int)ECOMBATGROUP_TYPE::LCA_SQUADRON) {

				if (units.size() > 0) {
					CombatUnit* carrier = units[0];

					for (int u = 0; u < g->GetUnits().size(); u++) {
						CombatUnit* unit = g->GetUnits()[u];

						if (unit->Type() >= UShip::FIGHTER ||
							unit->Type() <= UShip::LCA) {
							unit->SetCarrier(carrier);
							unit->SetRegion(carrier->GetRegion());
						}
					}
				}
			}
		}
	}

	return clone;
}

// +--------------------------------------------------------------------+

const char*
CombatGroup::GetOrdinal() const
{
	static char ordinal[16];

	int last_two_digits = id % 100;

	if (last_two_digits > 10 && last_two_digits < 20) {
		sprintf_s(ordinal, "ordinal.%d", last_two_digits);
		Text suffix = Game::GetText(ordinal);

		if (suffix != ordinal)
			sprintf_s(ordinal, "%d%s", id, suffix.data());
		else
			sprintf_s(ordinal, "%dth", id);
	}
	else {
		int last_digit = last_two_digits % 10;
		sprintf_s(ordinal, "ordinal.%d", last_digit);
		Text suffix = Game::GetText(ordinal);
		if (suffix != ordinal)
			sprintf_s(ordinal, "%d%s", id, suffix.data());
		else if (last_digit == 1)
			sprintf_s(ordinal, "%dst", id);
		else if (last_digit == 2)
			sprintf_s(ordinal, "%dnd", id);
		else if (last_digit == 3)
			sprintf_s(ordinal, "%drd", id);
		else
			sprintf_s(ordinal, "%dth", id);
	}

	return ordinal;
}

const char*
CombatGroup::GetDescription() const
{
	static char desc[256];
	static char name_desc[256];

	if (name.length())
		sprintf_s(name_desc, " \"%s\"", (const char*)name);
	else
		name_desc[0] = 0;

	switch (type) {
	case (int)ECOMBATGROUP_TYPE::FORCE:    
		strcpy_s(desc, (const char*)name); 
		break;

	case (int)ECOMBATGROUP_TYPE::FLEET:    
		sprintf_s(desc, "%s %s%s", GetOrdinal(), Game::GetText("CombatGroup.FLEET").data(), name_desc); 
		break;

	case (int)ECOMBATGROUP_TYPE::CARRIER_GROUP:  
		sprintf_s(desc, "%s %s%s", GetOrdinal(), Game::GetText("CombatGroup.CARRIER_GROUP").data(), name_desc); 
		break;

	case (int)ECOMBATGROUP_TYPE::BATTLE_GROUP:   
		sprintf_s(desc, "%s %s%s", GetOrdinal(), Game::GetText("CombatGroup.BATTLE_GROUP").data(), name_desc); 
		break;

	case (int)ECOMBATGROUP_TYPE::DESTROYER_SQUADRON:
		sprintf_s(desc, "%s %s%s", GetOrdinal(), Game::GetText("CombatGroup.DESTROYER_SQUADRON").data(), name_desc); 
		break;

	case (int)ECOMBATGROUP_TYPE::WING:        
		sprintf_s(desc, "%s %s%s", GetOrdinal(), Game::GetText("CombatGroup.WING").data(), name_desc); 
		break;

	case (int)ECOMBATGROUP_TYPE::ATTACK_SQUADRON:   
		sprintf_s(desc, "%s %s%s", GetOrdinal(), Game::GetText("CombatGroup.ATTACK_SQUADRON").data(), name_desc); 
		break;

	case (int)ECOMBATGROUP_TYPE::FIGHTER_SQUADRON:
		sprintf_s(desc, "%s %s%s", GetOrdinal(), Game::GetText("CombatGroup.FIGHTER_SQUADRON").data(), name_desc); 
		break;

	case (int)ECOMBATGROUP_TYPE::INTERCEPT_SQUADRON:   
		sprintf_s(desc, "%s %s%s", GetOrdinal(), Game::GetText("CombatGroup.INTERCEPT_SQUADRON").data(), name_desc); 
		break;

	case (int)ECOMBATGROUP_TYPE::LCA_SQUADRON:  
		sprintf_s(desc, "%s %s%s", GetOrdinal(), Game::GetText("CombatGroup.LCA_SQUADRON").data(), name_desc);
		break;

	case (int)ECOMBATGROUP_TYPE::BATTALION:       
		sprintf_s(desc, "%s %s%s", GetOrdinal(), Game::GetText("CombatGroup.BATTALION").data(), name_desc); 
		break;

	case (int)ECOMBATGROUP_TYPE::STATION:  
		sprintf_s(desc, "%s %s", Game::GetText("CombatGroup.STATION").data(), name.data()); 
		break;

	case (int)ECOMBATGROUP_TYPE::STARBASE:    
		sprintf_s(desc, "%s %d%s", Game::GetText("CombatGroup.STARBASE").data(), id, name_desc); 
		break;

	case (int)ECOMBATGROUP_TYPE::MINEFIELD:  
		sprintf_s(desc, "%s %s%s", GetOrdinal(), Game::GetText("CombatGroup.MINEFIELD").data(), name_desc); 
		break;

	case (int)ECOMBATGROUP_TYPE::BATTERY:  
		sprintf_s(desc, "%s %s%s", GetOrdinal(), Game::GetText("CombatGroup.BATTERY").data(), name_desc); 
		break;

	case (int)ECOMBATGROUP_TYPE::MISSILE:   
		sprintf_s(desc, "%s %s%s", GetOrdinal(), Game::GetText("CombatGroup.MISSILE").data(), name_desc); 
		break;

	case (int)ECOMBATGROUP_TYPE::C3I:                 
		sprintf_s(desc, "%s %s%s", GetOrdinal(), Game::GetText("CombatGroup.C3I").data(), name_desc); 
		break;

	case (int)ECOMBATGROUP_TYPE::COMM_RELAY:   
		sprintf_s(desc, "%s %s%s", GetOrdinal(), Game::GetText("CombatGroup.COMM_RELAY").data(), name_desc); 
		break;

	case (int)ECOMBATGROUP_TYPE::EARLY_WARNING:  
		sprintf_s(desc, "%s %s%s", GetOrdinal(), Game::GetText("CombatGroup.EARLY_WARNING").data(), name_desc);
		break;

	case (int)ECOMBATGROUP_TYPE::FWD_CONTROL_CTR: 
		sprintf_s(desc, "%s %s%s", GetOrdinal(), Game::GetText("CombatGroup.FWD_CONTROL_CTR").data(), name_desc);
		break;

	case (int)ECOMBATGROUP_TYPE::ECM:   
		sprintf_s(desc, "%s %s%s", GetOrdinal(), Game::GetText("CombatGroup.ECM").data(), name_desc); 
		break;

	case (int)ECOMBATGROUP_TYPE::SUPPORT: 
		sprintf_s(desc, "%s %s%s", GetOrdinal(), Game::GetText("CombatGroup.SUPPORT").data(), name_desc); 
		break;

	case (int)ECOMBATGROUP_TYPE::COURIER:  
		sprintf_s(desc, "%s %s%s", GetOrdinal(), Game::GetText("CombatGroup.COURIER").data(), name_desc); 
		break;

	case (int)ECOMBATGROUP_TYPE::SUPPLY:  
		sprintf_s(desc, "%s %s%s", GetOrdinal(), Game::GetText("CombatGroup.SUPPLY").data(), name_desc); 
		break;

	case (int)ECOMBATGROUP_TYPE::REPAIR:  
		sprintf_s(desc, "%s %s%s", GetOrdinal(), Game::GetText("CombatGroup.REPAIR").data(), name_desc); 
		break;

	case (int)ECOMBATGROUP_TYPE::MEDICAL:   
		sprintf_s(desc, "%s %s%s", GetOrdinal(), Game::GetText("CombatGroup.MEDICAL").data(), name_desc); 
		break;

	case (int)ECOMBATGROUP_TYPE::CIVILIAN:
	case (int)ECOMBATGROUP_TYPE::WAR_PRODUCTION:
	case (int)ECOMBATGROUP_TYPE::FACTORY:
	case (int)ECOMBATGROUP_TYPE::REFINERY:
	case (int)ECOMBATGROUP_TYPE::RESOURCE:   
		strcpy_s(desc, (const char*)name); 
		break;

	case (int)ECOMBATGROUP_TYPE::INFRASTRUCTURE:
	case (int)ECOMBATGROUP_TYPE::TRANSPORT:
	case (int)ECOMBATGROUP_TYPE::NETWORK:
	case (int)ECOMBATGROUP_TYPE::HABITAT:
	case (int)ECOMBATGROUP_TYPE::STORAGE:
	case (int)ECOMBATGROUP_TYPE::FREIGHT:
	case (int)ECOMBATGROUP_TYPE::PASSENGER:
	case (int)ECOMBATGROUP_TYPE::PRIVATE:   
		strcpy_s(desc, (const char*)name); 
		break;

	default:                   
		sprintf_s(desc, "%s%s", Game::GetText("CombatGroup.default").data(), name_desc); 
		break;
	}

	return desc;
}

const char*
CombatGroup::GetShortDescription() const
{
	static char desc[256];

	switch (type) {
	case (int)ECOMBATGROUP_TYPE::FORCE:
		strcpy_s(desc, (const char*)name); 
		break;

	case (int)ECOMBATGROUP_TYPE::FLEET:
		sprintf_s(desc, "%s %s", GetOrdinal(), Game::GetText("CombatGroup.abrv.FLEET").data());
		break;

	case (int)ECOMBATGROUP_TYPE::CARRIER_GROUP:       
		sprintf_s(desc, "%s %s", GetOrdinal(), Game::GetText("CombatGroup.abrv.CARRIER_GROUP").data()); 
		break;

	case (int)ECOMBATGROUP_TYPE::BATTLE_GROUP:
		sprintf_s(desc, "%s %s", GetOrdinal(), Game::GetText("CombatGroup.abrv.BATTLE_GROUP").data()); 
		break;

	case (int)ECOMBATGROUP_TYPE::DESTROYER_SQUADRON:
		sprintf_s(desc, "%s %s", GetOrdinal(), Game::GetText("CombatGroup.abrv.DESTROYER_SQUADRON").data()); 
		break;

	case (int)ECOMBATGROUP_TYPE::WING:         
		sprintf_s(desc, "%s %s", GetOrdinal(), Game::GetText("CombatGroup.abrv.WING").data()); 
		break;

	case (int)ECOMBATGROUP_TYPE::ATTACK_SQUADRON:     
		sprintf_s(desc, "%s %s", GetOrdinal(), Game::GetText("CombatGroup.abrv.ATTACK_SQUADRON").data()); 
		break;

	case (int)ECOMBATGROUP_TYPE::FIGHTER_SQUADRON:   
		sprintf_s(desc, "%s %s", GetOrdinal(), Game::GetText("CombatGroup.abrv.FIGHTER_SQUADRON").data()); 
		break;

	case (int)ECOMBATGROUP_TYPE::INTERCEPT_SQUADRON:   
		sprintf_s(desc, "%s %s", GetOrdinal(), Game::GetText("CombatGroup.abrv.INTERCEPT_SQUADRON").data()); 
		break;

	case (int)ECOMBATGROUP_TYPE::LCA_SQUADRON:  
		sprintf_s(desc, "%s %s", GetOrdinal(), Game::GetText("CombatGroup.abrv.LCA_SQUADRON").data()); 
		break;

	case (int)ECOMBATGROUP_TYPE::BATTALION:
		sprintf_s(desc, "%s %s", GetOrdinal(), Game::GetText("CombatGroup.abrv.BATTALION").data()); 
		break;

	case (int)ECOMBATGROUP_TYPE::STATION:  
		sprintf_s(desc, "%s %s", GetOrdinal(), Game::GetText("CombatGroup.abrv.STATION").data()); 
		break;

	case (int)ECOMBATGROUP_TYPE::STARBASE:  
		sprintf_s(desc, "%s %s", GetOrdinal(), Game::GetText("CombatGroup.abrv.STARBASE").data()); 
		break;

	case (int)ECOMBATGROUP_TYPE::MINEFIELD: 
		sprintf_s(desc, "%s %s", GetOrdinal(), Game::GetText("CombatGroup.abrv.MINEFIELD").data()); 
		break;

	case (int)ECOMBATGROUP_TYPE::BATTERY:  
		sprintf_s(desc, "%s %s", GetOrdinal(), Game::GetText("CombatGroup.abrv.BATTERY").data()); 
		break;

	case (int)ECOMBATGROUP_TYPE::C3I:  
		sprintf_s(desc, "%s %s", GetOrdinal(), Game::GetText("CombatGroup.abrv.C3I").data()); 
		break;

	case (int)ECOMBATGROUP_TYPE::COMM_RELAY:      
		sprintf_s(desc, "%s %s", GetOrdinal(), Game::GetText("CombatGroup.abrv.COMM_RELAY").data()); 
		break;

	case (int)ECOMBATGROUP_TYPE::EARLY_WARNING:  
		sprintf_s(desc, "%s %s", GetOrdinal(), Game::GetText("CombatGroup.abrv.EARLY_WARNING").data()); 
		break;

	case (int)ECOMBATGROUP_TYPE::FWD_CONTROL_CTR:    
		sprintf_s(desc, "%s %s", GetOrdinal(), Game::GetText("CombatGroup.abrv.FWD_CONTROL_CTR").data()); 
		break;

	case (int)ECOMBATGROUP_TYPE::ECM:  
		sprintf_s(desc, "%s %s", GetOrdinal(), Game::GetText("CombatGroup.abrv.ECM").data());
		break;

	case (int)ECOMBATGROUP_TYPE::SUPPORT:  
		sprintf_s(desc, "%s %s", GetOrdinal(), Game::GetText("CombatGroup.abrv.SUPPORT").data()); 
		break;

	case (int)ECOMBATGROUP_TYPE::COURIER:  
		sprintf_s(desc, "%s %s", GetOrdinal(), Game::GetText("CombatGroup.abrv.COURIER").data()); 
		break;

	case (int)ECOMBATGROUP_TYPE::MEDICAL:  
		sprintf_s(desc, "%s %s", GetOrdinal(), Game::GetText("CombatGroup.abrv.MEDICAL").data()); 
		break;

	case (int)ECOMBATGROUP_TYPE::SUPPLY:           
		sprintf_s(desc, "%s %s", GetOrdinal(), Game::GetText("CombatGroup.abrv.SUPPLY").data());
		break;

	case (int)ECOMBATGROUP_TYPE::REPAIR:   
		sprintf_s(desc, "%s %s", GetOrdinal(), Game::GetText("CombatGroup.abrv.REPAIR").data()); 
		break;

	case (int)ECOMBATGROUP_TYPE::CIVILIAN:
	case (int)ECOMBATGROUP_TYPE::WAR_PRODUCTION:
	case (int)ECOMBATGROUP_TYPE::FACTORY:
	case (int)ECOMBATGROUP_TYPE::REFINERY:
	case (int)ECOMBATGROUP_TYPE::RESOURCE:      
		strcpy_s(desc, (const char*)name);
		break;

	case (int)ECOMBATGROUP_TYPE::INFRASTRUCTURE:
	case (int)ECOMBATGROUP_TYPE::TRANSPORT:
	case (int)ECOMBATGROUP_TYPE::NETWORK:
	case (int)ECOMBATGROUP_TYPE::HABITAT:
	case (int)ECOMBATGROUP_TYPE::STORAGE:
	case (int)ECOMBATGROUP_TYPE::FREIGHT:
	case (int)ECOMBATGROUP_TYPE::PASSENGER:
	case (int)ECOMBATGROUP_TYPE::PRIVATE:         
		strcpy_s(desc, (const char*)name);
		break;

	default:
		sprintf_s(desc, "%s", Game::GetText("CombatGroup.abrv.default").data()); 
		break;
	}

	return desc;
}

// +--------------------------------------------------------------------+

double
CombatGroup::GetNextJumpTime() const
{
	double t = 0;

	ListIter<CombatUnit> unit = ((CombatGroup*)this)->units;
	while (++unit)
		if (unit->GetNextJumpTime() > t)
			t = unit->GetNextJumpTime();

	return t;
}

// +--------------------------------------------------------------------+

void
CombatGroup::MoveTo(const Point& loc)
{
	location = loc;
}

// +--------------------------------------------------------------------+

void
CombatGroup::SetAssignedSystem(const char* s)
{
	assigned_system = s;
	assigned_zone = 0;
	zone_lock = false;

	ListIter<CombatGroup> iter = components;
	while (++iter) {
		CombatGroup* g = iter.value();
		g->SetAssignedSystem(s);
	}
}

void
CombatGroup::SetAssignedZone(CombatZone* z)
{
	assigned_zone = z;

	if (!assigned_zone)
		zone_lock = false;

	ListIter<CombatGroup> iter = components;
	while (++iter) {
		CombatGroup* g = iter.value();
		g->SetAssignedZone(z);
	}
}

void
CombatGroup::ClearUnlockedZones()
{
	if (!zone_lock)
		assigned_zone = 0;

	ListIter<CombatGroup> iter = components;
	while (++iter) {
		CombatGroup* g = iter.value();
		g->ClearUnlockedZones();
	}
}

void
CombatGroup::SetZoneLock(bool lock)
{
	if (!assigned_zone)
		zone_lock = false;
	else
		zone_lock = lock;

	if (zone_lock)
		assigned_system = Text();

	ListIter<CombatGroup> iter = components;
	while (++iter) {
		CombatGroup* g = iter.value();
		g->SetZoneLock(lock);
	}
}

// +--------------------------------------------------------------------+

void
CombatGroup::SetIntelLevel(int n)
{
	if (n < (int)Intel::RESERVE || n >(int)Intel::TRACKED) return;

	enemy_intel = n;

	// if this group has been discovered, the entire
	// branch of the OOB tree must be exposed.  Otherwise,
	// no missions would ever be planned against this
	// combat group.

	if (n > Intel::SECRET) {
		CombatGroup* p = parent;
		while (p) {
			if (p->enemy_intel < (int)Intel::KNOWN)
				p->enemy_intel = (int)Intel::KNOWN;

			p = p->parent;
		}
	}
}

// +--------------------------------------------------------------------+

int
CombatGroup::CalcValue()
{
	int val = 0;

	ListIter<CombatUnit> unit = units;
	while (++unit)
		val += unit->GetValue();

	ListIter<CombatGroup> comp = components;
	while (++comp)
		val += comp->CalcValue();

	value = val;
	return value;
}

int
CombatGroup::CountUnits() const
{
	int n = 0;

	CombatGroup* g = (CombatGroup*)this;

	ListIter<CombatUnit> unit = g->units;
	while (++unit)
		n += unit->Count() - unit->DeadCount();

	CombatGroup* pThis = ((CombatGroup*)this);
	pThis->live_comp.clear();
	ListIter<CombatGroup> iter = g->components;
	while (++iter) {
		CombatGroup* comp = iter.value();

		if (!comp->IsReserve()) {
			int unit_count = comp->CountUnits();
			if (unit_count > 0)
				pThis->live_comp.append(comp);

			n += unit_count;
		}
	}

	return n;
}

// +--------------------------------------------------------------------+

void
CombatGroup::ClearAssignments()
{
	assignments.destroy();

	ListIter<CombatGroup> comp = components;
	while (++comp)
		comp->ClearAssignments();
}

// +--------------------------------------------------------------------+

CombatGroup*
CombatGroup::FindCarrier()
{
	CombatGroup* p = GetParent();

	while (p != 0 &&
		p->Type() != (int) ECOMBATGROUP_TYPE::CARRIER_GROUP &&
		p->Type() != (int)ECOMBATGROUP_TYPE::STATION &&
		p->Type() != (int)ECOMBATGROUP_TYPE::STARBASE)
		p = p->GetParent();

	if (p && p->GetUnits().size())
		return p;

	return 0;
}

CombatUnit*
CombatGroup::GetRandomUnit()
{
	CombatUnit* result = 0;
	List<CombatUnit>  live;

	ListIter<CombatUnit> unit = units;
	while (++unit) {
		if (unit->Count() - unit->DeadCount() > 0)
			live.append(unit.value());
	}

	if (live.size() > 0) {
		int ntries = 5;
		while (!result && ntries-- > 0) {
			int index = rand() % live.size();
			result = live[index];

			int ship_class = result->GetShipClass();
			if (ship_class >= UShip::CRUISER &&
				ship_class <= UShip::FARCASTER)
				result = 0;
		}
	}

	if (!result) {
		ListIter<CombatGroup> comp = components;
		while (++comp && !result) {
			CombatUnit* u = comp->GetRandomUnit();
			if (u)
				result = u;
		}
	}

	return result;
}

CombatUnit*
CombatGroup::GetFirstUnit()
{
	int tmp_index = unit_index;
	unit_index = 0;
	CombatUnit* result = GetNextUnit();
	unit_index = tmp_index;

	return result;
}

CombatUnit*
CombatGroup::GetNextUnit()
{
	if (units.size() > 0) {
		List<CombatUnit> live;

		ListIter<CombatUnit> unit = units;
		while (++unit) {
			if (unit->Count() - unit->DeadCount() > 0)
				live.append(unit.value());
		}

		if (live.size() > 0) {
			return live[unit_index++ % live.size()];
		}
	}

	if (components.size() > 0) {
		return components[unit_index % components.size()]->GetNextUnit();
	}

	return 0;
}

CombatUnit*
CombatGroup::FindUnit(const char* unitname)
{
	if (units.size() > 0) {
		ListIter<CombatUnit> iter = units;
		while (++iter) {
			CombatUnit* unit = iter.value();
			if (unit->Name() == unitname) {
				if (unit->Count() - unit->DeadCount() > 0)
					return unit;
				else
					return 0;
			}
		}
	}

	return 0;
}

void
CombatGroup::AssignRegion(Text rgn)
{
	region = rgn;

	ListIter<CombatGroup> comp = components;
	while (++comp)
		comp->AssignRegion(rgn);

	ListIter<CombatUnit> unit = units;
	while (++unit)
		unit->SetRegion(rgn);
}

// +--------------------------------------------------------------------+

static const char* group_name[] = {
	"",
	"force",
	"wing",
	"intercept_squadron",
	"fighter_squadron",
	"attack_squadron",
	"lca_squadron",
	"fleet",
	"destroyer_squadron",
	"battle_group",
	"carrier_group",
	"battalion",
	"minefield",
	"battery",
	"missile",
	"station",
	"starbase",
	"c3i",
	"comm_relay",
	"early_warning",
	"fwd_control_ctr",
	"ecm",
	"support",
	"courier",
	"medical",
	"supply",
	"repair",
	"civilian",
	"war_production",
	"factory",
	"refinery",
	"resource",
	"infrastructure",
	"transport",
	"network",
	"habitat",
	"storage",
	"non_com",
	"freight",
	"passenger",
	"private"
};

// +--------------------------------------------------------------------+

int
CombatGroup::TypeFromName(const char* type_name)
{
	for (int i = FORCE; i < PRIVATE; i++)
		if (!_stricmp(type_name, group_name[i]))
			return i;

	return 0;
}

const char*
CombatGroup::NameFromType(int type)
{
	return group_name[type];
}

// +--------------------------------------------------------------------+

/*int ShipClassFromName(const char* type_name)
{
	return Ship::ClassForName(type_name);
}*/

// +--------------------------------------------------------------------+

#define GET_DEF_BOOL(n) if (pdef->name()->value()==(#n)) GetDefBool((n),   pdef, filename)
#define GET_DEF_TEXT(n) if (pdef->name()->value()==(#n)) GetDefText((n),   pdef, filename)
#define GET_DEF_NUM(n)  if (pdef->name()->value()==(#n)) GetDefNumber((n), pdef, filename)
#define GET_DEF_VEC(n)  if (pdef->name()->value()==(#n)) GetDefVec((n),    pdef, filename)

/*CombatGroup*
CombatGroup::LoadOrderOfBattle(const char* filename, int team, Combatant* combatant)
{
	CombatGroup* force = 0;
	DataLoader* loader = DataLoader::GetLoader();
	BYTE* block;
	loader->LoadBuffer(filename, block, true);

	Parser parser(new BlockReader((const char*)block));
	Term* term = parser.ParseTerm();

	if (!term) {
		Print("ERROR: could not parse order of battle '%s'\n", filename);
		return 0;
	}
	else {
		TermText* file_type = term->isText();
		if (!file_type || file_type->value() != "ORDER_OF_BATTLE") {
			Print("ERROR: invalid Order of Battle file '%s'\n", filename);
			term->print(10);
			return 0;
		}
	}


	do {
		delete term; term = 0;
		term = parser.ParseTerm();

		if (term) {
			TermDef* def = term->isDef();
			if (def) {
				if (def->name()->value() == "group") {
					if (!def->term() || !def->term()->isStruct()) {
						Print("WARNING: group struct missing in '%s'\n", filename);
					}
					else {
						TermStruct* val = def->term()->isStruct();

						char  name[256];
						char  type[64];
						char  intel[64];
						char  region[64];
						char  system[64];
						char  parent_type[64];
						int   parent_id = 0;
						int   id = 0;
						int   iff = -1;
						Vec3  loc = Vec3(1.0e9f, 0.0f, 0.0f);

						List<CombatUnit>  unit_list;
						char              unit_name[64];
						char              unit_regnum[16];
						char              unit_design[64];
						char              unit_skin[64];
						int               unit_class = 0;
						int               unit_count = 1;
						int               unit_dead = 0;
						int               unit_damage = 0;
						int               unit_heading = 0;
						int               unit_index = 0;

						*name = 0;
						*type = 0;
						*intel = 0;
						*region = 0;
						*system = 0;
						*parent_type = 0;
						*unit_name = 0;
						*unit_regnum = 0;
						*unit_design = 0;
						*unit_skin = 0;

						strcpy_s(intel, "KNOWN");

						// all groups in this OOB default to the IFF of the main force
						if (force)
							iff = force->GetIFF();

						for (int i = 0; i < val->elements()->size(); i++) {
							TermDef* pdef = val->elements()->at(i)->isDef();
							if (pdef && (iff < 0 || team < 0 || iff == team)) {
								GET_DEF_TEXT(name);
							else GET_DEF_TEXT(type);
						else GET_DEF_TEXT(intel);
				else GET_DEF_TEXT(region);
			else GET_DEF_TEXT(system);
		else GET_DEF_VEC(loc);
else GET_DEF_TEXT(parent_type);
else GET_DEF_NUM(parent_id);
else GET_DEF_NUM(iff);
else GET_DEF_NUM(id);
								else GET_DEF_NUM(unit_index);

								else if ((iff == team || team < 0) && pdef->name()->value() == "unit") {
									if (!pdef->term() || !pdef->term()->isStruct()) {
										Print("WARNING: unit struct missing for group '%s' in '%s'\n", name, filename);
									}
									else {
										TermStruct* val = pdef->term()->isStruct();

										char unit_region[64];
										char design[256];
										Vec3 unit_loc = Vec3(1.0e9f, 0.0f, 0.0f);
										unit_count = 1;

										ZeroMemory(unit_region, sizeof(unit_region));
										ZeroMemory(design, sizeof(design));

										for (int i = 0; i < val->elements()->size(); i++) {
											TermDef* pdef = val->elements()->at(i)->isDef();
											if (pdef) {
												if (pdef->name()->value() == "name") {
													GetDefText(unit_name, pdef, filename);
												}
												else if (pdef->name()->value() == "regnum") {
													GetDefText(unit_regnum, pdef, filename);
												}
												else if (pdef->name()->value() == "region") {
													GetDefText(unit_region, pdef, filename);
												}
												else if (pdef->name()->value() == "loc") {
													GetDefVec(unit_loc, pdef, filename);
												}
												else if (pdef->name()->value() == "type") {
													char typestr[32];
													GetDefText(typestr, pdef, filename);
													unit_class = ShipDesign::ClassForName(typestr);
												}
												else if (pdef->name()->value() == "design") {
													GetDefText(unit_design, pdef, filename);
												}
												else if (pdef->name()->value() == "skin") {
													GetDefText(unit_skin, pdef, filename);
												}
												else if (pdef->name()->value() == "count") {
													GetDefNumber(unit_count, pdef, filename);
												}
												else if (pdef->name()->value() == "dead_count") {
													GetDefNumber(unit_dead, pdef, filename);
												}
												else if (pdef->name()->value() == "damage") {
													GetDefNumber(unit_damage, pdef, filename);
												}
												else if (pdef->name()->value() == "heading") {
													GetDefNumber(unit_heading, pdef, filename);
												}
											}
										}

										if (!ShipDesign::CheckName(unit_design)) {
											Print("ERROR: invalid design '%s' for unit '%s' in '%s'\n", unit_design, unit_name, filename);
											return 0;
										}

										CombatUnit* cu = new(__FILE__, __LINE__) CombatUnit(unit_name, unit_regnum, unit_class, unit_design, unit_count, iff);
										cu->SetRegion(unit_region);
										cu->SetSkin(unit_skin);
										cu->MoveTo(unit_loc);
										cu->Kill(unit_dead);
										cu->SetSustainedDamage(unit_damage);
										cu->SetHeading(unit_heading * DEGREES);
										unit_list.append(cu);
									}
									}
							}
						}  // elements

						if (iff >= 0 && (iff == team || team < 0)) {
							CombatGroup* parent_group = 0;

							if (force) {
								parent_group = force->FindGroup(TypeFromName(parent_type), parent_id);
							}

							CombatGroup* g = new(__FILE__, __LINE__)
								CombatGroup(TypeFromName(type), id, name, iff, Intel::IntelFromName(intel), parent_group);

							g->region = region;
							g->combatant = combatant;
							g->unit_index = unit_index;

							if (loc.x >= 1e9) {
								if (parent_group)
									g->location = parent_group->location;
								else
									g->location = Vec3(0, 0, 0);
							}
							else {
								g->location = loc;
							}

							if (unit_list.size()) {
								unit_list[0]->SetLeader(true);

								ListIter<CombatUnit> u = unit_list;
								while (++u) {
									u->SetCombatGroup(g);

									if (u->GetRegion().length() < 1) {
										u->SetRegion(g->GetRegion());
										u->MoveTo(g->Location());
									}

									if (parent_group &&
										(u->Type() == Ship::FIGHTER ||
											u->Type() == Ship::ATTACK)) {

										CombatUnit* carrier = 0;
										CombatGroup* p = parent_group;

										while (p && !carrier) {
											if (p->units.size() && p->units[0]->Type() == Ship::CARRIER) {
												carrier = p->units[0];
												u->SetCarrier(carrier);
												u->SetRegion(carrier->GetRegion());
											}

											p = p->parent;
										}
									}
								}

								g->units.append(unit_list);
							}

							if (!force)
								force = g;
						}  // iff == team?
					}     // group-struct
				}        // group
			}           // def
		}              // term
	} while (term);

	loader->ReleaseBuffer(block);
	Print("Order of Battle Loaded (%s).\n", force ? force->Name().data() : "unknown force");

	if (force)
		force->CalcValue();

	return force;
}

// +--------------------------------------------------------------------+

/*void
CombatGroup::MergeOrderOfBattle(BYTE* block, const char* filename, int team, Combatant* combatant, Campaign* campaign)
{
	CombatGroup* force = 0;

	Parser parser(new BlockReader((const char*)block));
	Term* term = parser.ParseTerm();

	if (!term) {
		Print("ERROR: could not parse order of battle '%s'\n", filename);
		return;
	}
	else {
		TermText* file_type = term->isText();
		if (!file_type || file_type->value() != "SAVEGAME") {
			Print("ERROR: invalid Save Game file '%s'\n", filename);
			term->print(10);
			return;
		}
	}


	do {
		delete term; term = 0;
		term = parser.ParseTerm();

		if (term) {
			TermDef* def = term->isDef();
			if (def) {
				if (def->name()->value() == "group") {
					if (!def->term() || !def->term()->isStruct()) {
						Print("WARNING: group struct missing in '%s'\n", filename);
					}
					else {
						TermStruct* val = def->term()->isStruct();

						char  name[256];
						char  type[64];
						char  intel[64];
						char  region[64];
						char  system[64];
						char  zone[64];
						bool  zone_locked = false;
						int   id = 0;
						int   iff = -1;
						int   sorties = -1;
						int   kills = -1;
						int   points = -1;
						Vec3  loc = Vec3(1.0e9f, 0.0f, 0.0f);

						List<CombatUnit>  unit_list;
						char              unit_name[64];
						char              unit_regnum[16];
						char              unit_design[64];
						int               unit_class = 0;
						int               unit_count = 1;
						int               unit_dead = 0;
						int               unit_damage = 0;
						int               unit_heading = 0;
						int               unit_index = 0;

						*name = 0;
						*type = 0;
						*intel = 0;
						*region = 0;
						*system = 0;
						*zone = 0;
						*unit_name = 0;
						*unit_regnum = 0;
						*unit_design = 0;

						strcpy_s(intel, "KNOWN");

						// all groups in this OOB default to the IFF of the main force
						if (force)
							iff = force->GetIFF();

						for (int i = 0; i < val->elements()->size(); i++) {
							TermDef* pdef = val->elements()->at(i)->isDef();
							if (pdef && (iff < 0 || team < 0 || iff == team)) {
								GET_DEF_TEXT(name);
							else GET_DEF_TEXT(type);
						else GET_DEF_TEXT(intel);
				else GET_DEF_TEXT(region);
			else GET_DEF_TEXT(system);
		else GET_DEF_TEXT(zone);
else GET_DEF_BOOL(zone_locked);
else GET_DEF_VEC(loc);
else GET_DEF_NUM(iff);
else GET_DEF_NUM(id);
								else GET_DEF_NUM(sorties);
								else GET_DEF_NUM(kills);
								else GET_DEF_NUM(points);
								else GET_DEF_NUM(unit_index);

								else if ((iff == team || team < 0) && pdef->name()->value() == "unit") {
									if (!pdef->term() || !pdef->term()->isStruct()) {
										Print("WARNING: unit struct missing for group '%s' in '%s'\n", name, filename);
									}
									else {
										TermStruct* val = pdef->term()->isStruct();

										char unit_region[64];
										char design[256];
										Vec3 unit_loc = Vec3(0.0f, 0.0f, 0.0f);
										unit_count = 1;

										ZeroMemory(unit_region, sizeof(unit_region));
										ZeroMemory(design, sizeof(design));

										for (int i = 0; i < val->elements()->size(); i++) {
											TermDef* pdef = val->elements()->at(i)->isDef();
											if (pdef) {
												if (pdef->name()->value() == "name") {
													GetDefText(unit_name, pdef, filename);
												}
												else if (pdef->name()->value() == "regnum") {
													GetDefText(unit_regnum, pdef, filename);
												}
												else if (pdef->name()->value() == "region") {
													GetDefText(unit_region, pdef, filename);
												}
												else if (pdef->name()->value() == "loc") {
													GetDefVec(unit_loc, pdef, filename);
												}
												else if (pdef->name()->value() == "type") {
													char typestr[32];
													GetDefText(typestr, pdef, filename);
													unit_class = ShipDesign::ClassForName(typestr);
												}
												else if (pdef->name()->value() == "design") {
													GetDefText(unit_design, pdef, filename);
												}
												else if (pdef->name()->value() == "count") {
													GetDefNumber(unit_count, pdef, filename);
												}
												else if (pdef->name()->value() == "dead_count") {
													GetDefNumber(unit_dead, pdef, filename);
												}
												else if (pdef->name()->value() == "damage") {
													GetDefNumber(unit_damage, pdef, filename);
												}
												else if (pdef->name()->value() == "heading") {
													GetDefNumber(unit_heading, pdef, filename);
												}
											}
										}

										if (!ShipDesign::CheckName(unit_design)) {
											Print("ERROR: invalid design '%s' for unit '%s' in '%s'\n", unit_design, unit_name, filename);
											return;
										}

										if (force) {
											CombatUnit* cu = new CombatUnit(unit_name, unit_regnum, unit_class, unit_design, unit_count, iff);
											cu->SetRegion(unit_region);
											cu->MoveTo(unit_loc);
											cu->Kill(unit_dead);
											cu->SetSustainedDamage(unit_damage);
											cu->SetHeading(unit_heading * DEGREES);
											unit_list.append(cu);
										}
									}
									}
							}
						}  // elements

						if (iff >= 0 && (iff == team || team < 0)) {
							// have we found the force group we are looking for yet?
							if (!force && !_stricmp(name, combatant->Name())) {
								force = combatant->GetForce();
							}

							else {
								if (!force)
									continue;

								// if we already have a force, and we find a second one, 
								// it must be the start of a different combatant.
								// So don't process any further:
								if (TypeFromName(type) == CombatGroup::FORCE) {
									break;
								}
							}

							CombatGroup* g = force->FindGroup(TypeFromName(type), id);

							if (!g) {
								::Print("WARNING: unexpected combat group %s %d '%s' in '%s'\n", type, id, name, filename);
								continue;
							}

							g->region = region;
							g->combatant = combatant;
							g->location = loc;
							g->enemy_intel = Intel::IntelFromName(intel);
							g->unit_index = unit_index;

							if (*zone) {
								CombatZone* combat_zone = campaign->GetZone(zone);

								if (combat_zone) {
									g->SetAssignedZone(combat_zone);
									g->SetZoneLock(zone_locked);
								}
								else {
									::Print("WARNING: could not find combat zone '%s' for group %s %d '%s' in '%s'\n", zone, type, id, name, filename);
								}
							}
							else if (*system) {
								g->SetAssignedSystem(system);
							}

							if (sorties >= 0) g->SetSorties(sorties);
							if (kills >= 0) g->SetKills(kills);
							if (points >= 0) g->SetPoints(points);

							if (unit_list.size()) {
								ListIter<CombatUnit> u_iter = unit_list;
								while (++u_iter) {
									CombatUnit* load_unit = u_iter.value();
									CombatUnit* u = g->FindUnit(load_unit->Name());

									if (u) {
										if (load_unit->GetRegion().length() > 0) {
											u->SetRegion(load_unit->GetRegion());
											u->MoveTo(load_unit->Location());
										}
										else {
											u->SetRegion(g->GetRegion());
											u->MoveTo(g->Location());
										}
										u->SetDeadCount(load_unit->DeadCount());
										u->SetSustainedDamage(load_unit->GetSustainedDamage());
										u->SetHeading(load_unit->GetHeading());
									}
								}

								unit_list.destroy();
							}

							if (!force)
								force = g;
						}  // iff == team?
					}     // group-struct
				}        // group
			}           // def
		}              // term
	} while (term);

	Print("Order of Battle Loaded (%s).\n", force ? force->Name().data() : "unknown force");

	if (force)
		force->CalcValue();
}*/

// +--------------------------------------------------------------------+

Text FormatNumber(double n)
{
	char buffer[64];

	if (fabs(n) < 1000)
		sprintf_s(buffer, "%d", (int)n);

	else if (fabs(n) < 1e6) {
		int nn = (int)n / 1000;
		sprintf_s(buffer, "%de3", nn);
	}

	else
		sprintf_s(buffer, "%g", n);

	return buffer;
}

/*void
SaveCombatUnit(FILE* f, CombatUnit* u)
{
	int type = u->Type();

	if (type == 0 && u->GetDesign())
		type = u->GetDesign()->type;

	fprintf(f, "\n unit: {");
	fprintf(f, " name: \"%s\",", u->Name().data());
	fprintf(f, " type: \"%s\",", Ship::ClassName(type));
	fprintf(f, " design: \"%s\",", u->DesignName().data());

	if (u->Count() > 1) {
		fprintf(f, " count: %d,", u->Count());
	}
	else {
		fprintf(f, " regnum:\"%s\",", u->Registry().data());
	}

	if (u->GetRegion().length() > 0) {
		fprintf(f, " region:\"%s\",", u->GetRegion().data());

		Text x = FormatNumber(u->Location().x);
		Text y = FormatNumber(u->Location().y);
		Text z = FormatNumber(u->Location().z);

		fprintf(f, " loc:(%s, %s, %s),", x.data(), y.data(), z.data());
	}

	fprintf(f, " dead_count: %d, damage: %d, heading: %d },",
		(int)u->DeadCount(),
		(int)u->GetSustainedDamage(),
		(int)(u->GetHeading() / DEGREES));
}

void
SaveCombatGroup(FILE* f, CombatGroup* g)
{
	fprintf(f, "group: {");
	fprintf(f, " type: %s,", CombatGroup::NameFromType(g->Type()));
	fprintf(f, " id: %d,", g->GetID());
	fprintf(f, " name: \"%s\",", g->Name().data());
	fprintf(f, " intel: %s,", Intel::NameFromIntel(g->IntelLevel()));
	fprintf(f, " iff: %d,", g->GetIFF());
	fprintf(f, " unit_index: %d,", g->UnitIndex());

	if (g->GetRegion().length()) {
		fprintf(f, " region:\"%s\",", g->GetRegion().data());
	}

	if (g->GetAssignedSystem().length()) {
		fprintf(f, " system: \"%s\",", g->GetAssignedSystem().data());
	}

	if (g->GetAssignedZone()) {
		fprintf(f, " zone: \"%s\",", g->GetAssignedZone()->Name().data());
		if (g->IsZoneLocked()) {
			fprintf(f, " zone_locked: true,");
		}
	}

	Text x = FormatNumber(g->Location().x);
	Text y = FormatNumber(g->Location().y);
	Text z = FormatNumber(g->Location().z);

	fprintf(f, " loc: (%s, %s, %s),", x.data(), y.data(), z.data());

	CombatGroup* parent = g->GetParent();
	if (parent) {
		fprintf(f, " parent_type:%s,", CombatGroup::NameFromType(parent->Type()));
		fprintf(f, " parent_id:%d,", parent->GetID());
	}

	fprintf(f, " sorties: %d,", g->Sorties());
	fprintf(f, " kills: %d,", g->Kills());
	fprintf(f, " points: %d,", g->Points());

	ListIter<CombatUnit> u = g->GetUnits();
	while (++u) {
		SaveCombatUnit(f, u.value());
	}

	fprintf(f, " }\n");

	ListIter<CombatGroup> c = g->GetComponents();
	while (++c) {
		SaveCombatGroup(f, c.value());
	}
}

void
CombatGroup::SaveOrderOfBattle(const char* filename, CombatGroup* force)
{
	FILE* f;
	::fopen_s(&f, filename, "a+");

	if (f) {
		SaveCombatGroup(f, force);
		fprintf(f, "\n");
		fclose(f);
	}
}*/

