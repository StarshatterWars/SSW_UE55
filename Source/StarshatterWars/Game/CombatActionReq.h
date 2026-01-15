/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         CombatActionReq.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	A planned action (mission/story/strategy) in a dynamic campaign.
*/

#pragma once

#include "CoreMinimal.h"
#include "SSWGameInstance.h"

class Combatant;
/**
 * 
 */
class STARSHATTERWARS_API CombatActionReq
{
public:
	
	static const char* TYPENAME() { return "CombatActionReq"; }

	CombatActionReq(int a, int s, bool n = false)
		: action(a), stat(s), notreq(n), c1(0), c2(0), comp(0), score(0), intel(0) { }

	CombatActionReq(Combatant* a1, Combatant* a2, int comparison, int value)
		: action(0), stat(0), notreq(0), c1(a1), c2(a2), group_type(0), group_id(0),
		comp(comparison), score(value), intel(0) { }

	CombatActionReq(Combatant* a1, int gtype, int gid, int comparison, int value, int intel_level = 0)
		: action(0), stat(0), notreq(0), c1(a1), c2(0), group_type(gtype), group_id(gid),
		comp(comparison), score(value), intel(intel_level) { }

	static int CompFromName(const char* sym);

	int   action;
	int   stat;
	bool  notreq;

	Combatant* c1;
	Combatant* c2;
	int         comp;
	int         score;
	int         intel;
	int         group_type;
	int         group_id;
};
