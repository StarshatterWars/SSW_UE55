/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         ZoneForce.h
	AUTHOR:       Carlos Bott


	OVERVIEW
	========
	Zone Force claSS
*/

#pragma once

#include "CoreMinimal.h"
#include "../Foundation/List.h"
#include "../System/SSWGameInstance.h"


class CombatGroup;

/**
 * 
 */
class STARSHATTERWARS_API ZoneForce
{
public:
	ZoneForce();
	~ZoneForce();

	ZoneForce(int i);

	int                  GetIFF() { return iff; }
	List<CombatGroup>& GetGroups() { return groups; }
	List<CombatGroup>& GetTargetList() { return target_list; }
	List<CombatGroup>& GetDefendList() { return defend_list; }

	void                 AddGroup(CombatGroup* g);
	void                 RemoveGroup(CombatGroup* g);
	bool                 HasGroup(CombatGroup* g);

	int                  GetNeed(int group_type) const;
	void                 SetNeed(int group_type, int needed);
	void                 AddNeed(int group_type, int needed);

private:
	// attributes:
	int                  iff;
	List<CombatGroup>    groups;
	List<CombatGroup>    defend_list;
	List<CombatGroup>    target_list;
	int                  need[8];
};
