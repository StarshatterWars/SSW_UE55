/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         CampaignPlan.h
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


#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Tickable.h"
#include "CampaignPlan.generated.h"

class ACampaign;
/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UCampaignPlan : public UObject, public FTickableGameObject
{
	GENERATED_BODY()
	
public:	
	static const char* TYPENAME() { return "CampaignPlan"; }

	UCampaignPlan();
	UCampaignPlan(ACampaign* c) : campaign(c), exec_time(-1e6) { }
	virtual ~UCampaignPlan() { }

	int operator == (const UCampaignPlan& p) const { return this == &p; }

	// operations:
	virtual void      ExecFrame() { }
	virtual void      SetLockout(int seconds) { }

	void Tick(float DeltaTime) override;
	bool IsTickable() const override;
	bool IsTickableInEditor() const override;
	bool IsTickableWhenPaused() const override;
	TStatId GetStatId() const override;

	UWorld* GetWorld() const override;

protected:
	ACampaign* campaign;
	double     exec_time;

};
