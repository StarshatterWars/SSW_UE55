/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         CampaignPlanMission.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	CampaignPlanMission generates missions and mission
	info for the player's combat group as part of a
	dynamic campaign.
*/

#pragma once

#include "CoreMinimal.h"
#include "CampaignPlan.h"
#include "Types.h"
#include "List.h"
#include "CampaignPlanMission.generated.h"


// +--------------------------------------------------------------------+

class CampaignMissionRequest;
class CombatGroup;
class CombatUnit;
class CombatZone;
class ZoneForce;
class UCampaign;

// +--------------------------------------------------------------------+
/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UCampaignPlanMission : public UCampaignPlan
{
	GENERATED_BODY()
	
	static const char* TYPENAME() { return "CampaignPlanMission"; }

	UCampaignPlanMission();
	UCampaignPlanMission(UCampaign* c); 

	// operations:
	virtual void   ExecFrame();

protected:
	virtual void SelectStartTime();

	CampaignMissionRequest* PlanCampaignMission();
	CampaignMissionRequest* PlanStrategicMission();
	CampaignMissionRequest* PlanRandomStarshipMission();
	CampaignMissionRequest* PlanRandomFighterMission();
	CampaignMissionRequest* PlanStarshipMission();
	CampaignMissionRequest* PlanFighterMission();

	CombatGroup* player_group;
	int            start;
	int            slot;	
	
};
