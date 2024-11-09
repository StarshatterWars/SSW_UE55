/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         CampaignPlanStrategic.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	CampaignPlanStrategic prioritizes targets and defensible
	allied forces as the first step in force tasking.  This
	algorithm computes which enemy resources are most important
	to attack, based on the AI value of each combat group, and
	strategic weighting factors that help shape the strategy
	to the objectives for the current campaign.
*/

#pragma once

#include "CoreMinimal.h"
#include "CampaignPlan.h"
#include "CampaignPlanStrategic.generated.h"

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UCampaignPlanStrategic : public UCampaignPlan
{
	GENERATED_BODY()
	
	
};
