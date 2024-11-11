/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         CampaignMissionRequest.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Campaign Mission Request 
*/

#pragma once

#include "CoreMinimal.h"
#include "../Foundation/Types.h"
#include "../Foundation/Geometry.h"
#include "../Foundation/List.h"
#include "../Foundation/Text.h"

// +-------------------------------------------------------------------- +

class UCampaign;
class CombatGroup;
class CombatUnit;
class CombatZone;
class ZoneForce;
class Mission;
class MissionElement;
class MissionInfo;

// +--------------------------------------------------------------------+
/**
 * 
 */
class STARSHATTERWARS_API CampaignMissionRequest
{
public:
public:
	static const char* TYPENAME() { return "CampaignMissionRequest"; }

	CampaignMissionRequest();
	CampaignMissionRequest(UCampaign* c, int type, int start,
		CombatGroup* primary, CombatGroup* tgt = 0);

	UCampaign* GetCampaign() { return campaign; }
	int Type() { return type; }
	int OpposingType() { return opp_type; }
	int StartTime() { return start; }
	CombatGroup* GetPrimaryGroup() { return primary_group; }
	CombatGroup* GetSecondaryGroup() { return secondary_group; }
	CombatGroup* GetObjective() { return objective; }

	bool IsLocSpecified() { return use_loc; }
	const Text& RegionName() { return region; }
	Point Location() { return location; }
	const Text& Script() { return script; }

	void              SetType(int t) { type = t; }
	void              SetOpposingType(int t) { opp_type = t; }
	void              SetStartTime(int s) { start = s; }
	void              SetPrimaryGroup(CombatGroup* g) { primary_group = g; }
	void              SetSecondaryGroup(CombatGroup* g) { secondary_group = g; }
	void              SetObjective(CombatGroup* g) { objective = g; }

	void              SetRegionName(const char* rgn) { region = rgn;   use_loc = true; }
	void              SetLocation(const Point& loc) { location = loc; use_loc = true; }
	void              SetScript(const char* s) { script = s; }

private:
	UCampaign* campaign;

	int               type;             // type of mission
	int               opp_type;         // opposing mission type
	int               start;            // start time
	CombatGroup* primary_group;    // player's group
	CombatGroup* secondary_group;  // optional support group
	CombatGroup* objective;        // target or ward

	bool              use_loc;          // use the specified location
	Text              region;
	Point             location;
	Text              script;
};
