/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         MissionInfo.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Campaign defines a strategic military scenario.
*/

#pragma once

#include "CoreMinimal.h"
#include "../Foundation/Text.h"
#include "../Foundation/Geometry.h"
#include "../Foundation/Color.h"
#include "../Foundation/Term.h"
#include "../Foundation/List.h"
#include "../Foundation/DataLoader.h"
#include "../System/SSWGameInstance.h"

// +--------------------------------------------------------------------+

class UCampaign;
class Mission;

// +--------------------------------------------------------------------

/**
 * 
 */
class STARSHATTERWARS_API MissionInfo
{
public:
	static const char* TYPENAME() { return "MissionInfo"; }
		
	MissionInfo();
	~MissionInfo();

	int operator == (const MissionInfo& m) const { return id == m.id; }
	int operator <  (const MissionInfo& m) const { return id < m.id; }
	int operator <= (const MissionInfo& m) const { return id <= m.id; }

	bool     IsAvailable();

	int      id;
	Text     name;
	Text     player_info;
	Text     description;
	Text     system;
	Text     region;
	Text     script;
	int      start;
	int      type;

	int      min_rank;
	int      max_rank;
	int      action_id;
	int      action_status;
	int      exec_once;
	int      start_before;
	int      start_after;

	Mission* mission;
};


