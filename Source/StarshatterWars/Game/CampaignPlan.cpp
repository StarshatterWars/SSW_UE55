/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         CampaignPlan.cpp
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	CampaignPlan defines the interface for all campaign
	planning algorithms.  Known subclasses:
	CampaignPlanStrategic  - strategic planning
	CampaignPlanAssignment - logistics planning
	CampaignPlanMission    - mission planning
	CampaignPlanMovement   - starship movement
	CampaignPlanEvent      - scripted events
*/


#include "CampaignPlan.h"

UCampaignPlan::UCampaignPlan()
{

}

void UCampaignPlan::Tick(float DeltaTime)
{

}

bool UCampaignPlan::IsTickable() const
{
	return false;
}

bool UCampaignPlan::IsTickableInEditor() const
{
	return false;
}

bool UCampaignPlan::IsTickableWhenPaused() const
{
	return false;
}

TStatId UCampaignPlan::GetStatId() const
{
	return TStatId();
}

UWorld* UCampaignPlan::GetWorld() const
{
	return GetOuter()->GetWorld();
}
