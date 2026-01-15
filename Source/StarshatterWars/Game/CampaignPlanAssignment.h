/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         CampaignPlanAssignment.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	CampaignPlanAssignment creates combat assignments for
	assets within each combat zone as the third step in
	force tasking.
*/

#pragma once

#include "CoreMinimal.h"
#include "CampaignPlan.h"
#include "Types.h"
#include "List.h"
#include "CampaignPlanAssignment.generated.h"

// +--------------------------------------------------------------------+

class Combatant;
class CombatGroup;
class CombatUnit;
class CombatZone;
class UCampaign;

// +--------------------------------------------------------------------+

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UCampaignPlanAssignment : public UCampaignPlan
{
	GENERATED_BODY()

public:
	static const char* TYPENAME() { return "CampaignPlanAssignment"; }

	UCampaignPlanAssignment();
	UCampaignPlanAssignment(UCampaign* c);
	~UCampaignPlanAssignment() { }

	// operations:
	virtual void   ExecFrame();

protected:
	virtual void   ProcessCombatant(Combatant* c);
	virtual void   ProcessZone(Combatant* c, CombatZone* zone);
	virtual void   BuildZoneList(CombatGroup* g, CombatZone* zone, List<CombatGroup>& list);
	virtual void   BuildAssetList(const int* pref, List<CombatGroup>& avail, List<CombatGroup>& assets);
	
};
