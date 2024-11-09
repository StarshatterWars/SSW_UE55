/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         CampaignPlanEvent.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	CampaignPlanEvent generates simulated combat
	events based on a statistical analysis of the
	combatants within the context of a dynamic
	campaign.
*/

#pragma once

#include "CoreMinimal.h"
#include "CampaignPlan.h"
#include "../Foundation/Types.h"
#include "../Foundation/Text.h"
#include "CampaignPlanEvent.generated.h"

// +--------------------------------------------------------------------+

class CombatAction;
class CombatAssignment;
class CombatEvent;
class CombatGroup;
class CombatUnit;
class CombatZone;
class ZoneForce;

// +--------------------------------------------------------------------+

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UCampaignPlanEvent : public UCampaignPlan
{
	GENERATED_BODY()
	
	static const char* TYPENAME() { return "CampaignPlanEvent"; }

	UCampaignPlanEvent();
	UCampaignPlanEvent(ACampaign* c);
	~UCampaignPlanEvent();

	// operations:
	virtual void   ExecFrame();
	virtual void   SetLockout(int seconds);

	virtual bool   ExecScriptedEvents();
	virtual bool   ExecStatisticalEvents();

protected:
	virtual void   ProsecuteKills(CombatAction* action);

	virtual CombatAssignment* ChooseAssignment(CombatGroup* c);
	virtual bool   CreateEvent(CombatAssignment* a);

	virtual CombatEvent* CreateEventDefend(CombatAssignment* a);
	virtual CombatEvent* CreateEventFighterAssault(CombatAssignment* a);
	virtual CombatEvent* CreateEventFighterStrike(CombatAssignment* a);
	virtual CombatEvent* CreateEventFighterSweep(CombatAssignment* a);
	virtual CombatEvent* CreateEventStarship(CombatAssignment* a);

	virtual bool IsFriendlyAssignment(CombatAssignment* a);
	virtual bool Success(CombatAssignment* a);
	virtual Text GetTeamName(CombatGroup* g);

	// attributes:
	int            event_time;
	
	
};
