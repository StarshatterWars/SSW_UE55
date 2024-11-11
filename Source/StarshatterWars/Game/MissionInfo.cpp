/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         MissionInfo.cpp
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Campaign defines a strategic military scenario.
*/


#include "MissionInfo.h"
#include "Campaign.h"
#include "PlayerData.h"
#include "CombatGroup.h"

const int TIME_NEVER = (int)1e9;

MissionInfo::MissionInfo()
{
	mission = 0;
	start = 0;
	type = 0;
	id = 0;
	min_rank = 0;
	max_rank = 0;
	
	action_id = 0;
	action_status = 0;
	exec_once = 0;

	start_before = TIME_NEVER;
	start_after = 0;
}

MissionInfo::~MissionInfo()
{
	//delete mission;
}

bool MissionInfo::IsAvailable()
{
	UCampaign* campaign = UCampaign::GetCampaign();
	PlayerData* player = PlayerData::GetCurrentPlayer();
	CombatGroup* player_group = campaign->GetPlayerGroup();

	if (campaign->GetTime() < start_after)
		return false;

	if (campaign->GetTime() > start_before)
		return false;

	if (region.length() && player_group->GetRegion() != region)
		return false;

	if (min_rank && player->Rank() < min_rank)
		return false;

	if (max_rank && player->Rank() > max_rank)
		return false;

	if (exec_once < 0)
		return false;

	if (exec_once > 0)
		exec_once = -1;

	return true;
}
