/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         Combatant.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	One side in a military conflict
*/


#pragma once

#include "CoreMinimal.h"
#include "../Foundation/Types.h"
#include "../Foundation/Geometry.h"
#include "../Foundation/Text.h"
#include "../Foundation/List.h"
#include "../System/SSWGameInstance.h"

// +--------------------------------------------------------------------+

class ACampaign;
class CombatGroup;
class CombatUnit;
class Mission;

// +--------------------------------------------------------------------+

/**
 * 
 */
class STARSHATTERWARS_API Combatant
{
public:
	
	static const char* TYPENAME() { return "Combatant"; }

	Combatant(const char* com_name, const char* file_name, int iff);
	Combatant(const char* com_name, CombatGroup* force);
	~Combatant();

	// operations:

	// accessors:
	const char* Name()           const { return name; }
	int                     GetIFF()         const { return iff; }
	int                     Score()          const { return score; }
	const char* GetDescription() const { return name; }
	CombatGroup* GetForce() { return force; }
	CombatGroup* FindGroup(int type, int id = -1);
	List<CombatGroup>& GetTargetList() { return target_list; }
	List<CombatGroup>& GetDefendList() { return defend_list; }
	List<Mission>& GetMissionList() { return mission_list; }

	void                    AddMission(Mission* m);
	void                    SetScore(int points) { score = points; }
	void                    AddScore(int points) { score += points; }

	double                  GetTargetStratFactor(int type);
	void                    SetTargetStratFactor(int type, double f);

private:
	Text                    name;
	int                     iff;
	int                     score;

	CombatGroup* force;
	List<CombatGroup>       target_list;
	List<CombatGroup>       defend_list;
	List<Mission>           mission_list;

	double                  target_factor[8];

};
