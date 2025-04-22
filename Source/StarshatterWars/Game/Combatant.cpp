/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         Combatant.cpp
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	One side in a military conflict
*/



#include "Combatant.h"
#include "CombatGroup.h"
#include "Mission.h"

#include "../System/Game.h"

// +--------------------------------------------------------------------+

static void SetCombatant(CombatGroup* g, Combatant* c)
{
	if (!g) return;

	g->SetCombatant(c);

	ListIter<CombatGroup> iter = g->GetComponents();
	while (++iter)
		SetCombatant(iter.value(), c);
}

// +--------------------------------------------------------------------+

Combatant::Combatant(const char* com_name, const char* fname, int team)
	: name(com_name), iff(team), score(0), force(0)
{
	for (int i = 0; i < 6; i++)
		target_factor[i] = 1;

	target_factor[2] = 1000;

	if (fname)
		force = CombatGroup::LoadOrderOfBattle(fname, iff, this);
}

Combatant::Combatant(const char* com_name, CombatGroup* f)
	: name(com_name), iff(0), score(0), force(f)
{
	for (int i = 0; i < 6; i++)
		target_factor[i] = 1;

	target_factor[2] = 1000;

	if (force) {
		SetCombatant(force, this);
		iff = force->GetIFF();
	}
}

// +--------------------------------------------------------------------+

Combatant::~Combatant()
{
	mission_list.clear();
	target_list.clear();
	defend_list.clear();
	delete force;
}

// +--------------------------------------------------------------------+

CombatGroup*
Combatant::FindGroup(int type, int id)
{
	if (force)
		return force->FindGroup(type, id);

	return 0;
}

// +--------------------------------------------------------------------+

void
Combatant::AddMission(Mission* mission)
{
	mission_list.append(mission);
}

// +--------------------------------------------------------------------+

double
Combatant::GetTargetStratFactor(int type)
{
	switch (type) {
	case (int8) ECOMBATGROUP_TYPE::FLEET:
	case (int8) ECOMBATGROUP_TYPE::CARRIER_GROUP:
	case (int8) ECOMBATGROUP_TYPE::BATTLE_GROUP:
	case (int8) ECOMBATGROUP_TYPE::DESTROYER_SQUADRON: 
		return target_factor[0];

	case (int8) ECOMBATGROUP_TYPE::WING:
	case (int8) ECOMBATGROUP_TYPE::ATTACK_SQUADRON:
	case (int8) ECOMBATGROUP_TYPE::INTERCEPT_SQUADRON:
	case (int8) ECOMBATGROUP_TYPE::FIGHTER_SQUADRON:   
		return target_factor[1];

	case (int8) ECOMBATGROUP_TYPE::BATTERY:
	case (int8) ECOMBATGROUP_TYPE::MISSILE:  
		return target_factor[2];

	case (int8) ECOMBATGROUP_TYPE::BATTALION:
	case (int8) ECOMBATGROUP_TYPE::STARBASE:
	case (int8) ECOMBATGROUP_TYPE::C3I:
	case (int8) ECOMBATGROUP_TYPE::COMM_RELAY:
	case (int8) ECOMBATGROUP_TYPE::EARLY_WARNING:
	case (int8) ECOMBATGROUP_TYPE::FWD_CONTROL_CTR:
	case (int8) ECOMBATGROUP_TYPE::ECM:       
		return target_factor[3];

	case (int8) ECOMBATGROUP_TYPE::SUPPORT:
	case (int8) ECOMBATGROUP_TYPE::COURIER:
	case (int8) ECOMBATGROUP_TYPE::MEDICAL:
	case (int8) ECOMBATGROUP_TYPE::SUPPLY:
	case (int8) ECOMBATGROUP_TYPE::REPAIR:  
		return target_factor[4];
	}

	return target_factor[5];
}

// +--------------------------------------------------------------------+

void
Combatant::SetTargetStratFactor(int type, double factor)
{
	switch (type) {
	case (int8) ECOMBATGROUP_TYPE::FLEET:
	case (int8) ECOMBATGROUP_TYPE::CARRIER_GROUP:
	case (int8) ECOMBATGROUP_TYPE::BATTLE_GROUP:
	case (int8) ECOMBATGROUP_TYPE::DESTROYER_SQUADRON: 
		target_factor[0] = factor;
		break;

	case (int8) ECOMBATGROUP_TYPE::WING:
	case (int8) ECOMBATGROUP_TYPE::ATTACK_SQUADRON:
	case (int8) ECOMBATGROUP_TYPE::INTERCEPT_SQUADRON:
	case (int8) ECOMBATGROUP_TYPE::FIGHTER_SQUADRON: 
		target_factor[1] = factor;
		break;

	case (int8) ECOMBATGROUP_TYPE::BATTALION:
	case (int8) ECOMBATGROUP_TYPE::STARBASE:
	case (int8) ECOMBATGROUP_TYPE::BATTERY:
	case (int8) ECOMBATGROUP_TYPE::MISSILE: 
		target_factor[2] = factor;
		break;

	case (int8) ECOMBATGROUP_TYPE::C3I:
	case (int8) ECOMBATGROUP_TYPE::COMM_RELAY:
	case (int8) ECOMBATGROUP_TYPE::EARLY_WARNING:
	case (int8) ECOMBATGROUP_TYPE::FWD_CONTROL_CTR:
	case (int8) ECOMBATGROUP_TYPE::ECM:  
		target_factor[3] = factor;
		break;

	case (int8) ECOMBATGROUP_TYPE::SUPPORT:
	case (int8) ECOMBATGROUP_TYPE::COURIER:
	case (int8) ECOMBATGROUP_TYPE::MEDICAL:
	case (int8) ECOMBATGROUP_TYPE::SUPPLY:
	case (int8) ECOMBATGROUP_TYPE::REPAIR:     
		target_factor[4] = factor;
		break;

	default:    
		target_factor[5] = factor;
		break;
	}
}