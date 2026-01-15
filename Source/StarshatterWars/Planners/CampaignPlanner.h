// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "CampaignTickContext.h"
#include "CampaignScheduler.h"

class UCampaignSubsystem;

// -------------------------------------------------
// Planner interface (pure C++)
// -------------------------------------------------
class ICampaignPlanner
{
public:
	virtual ~ICampaignPlanner() = default;

	virtual const TCHAR* GetName() const = 0;
	virtual void Reset(int64 NowSeconds) = 0;
	virtual void TickCampaign(const FCampaignTickContext& Ctx) = 0;
};

// -------------------------------------------------
// Base class with cadence gating
// -------------------------------------------------
class FCampaignPlannerBase : public ICampaignPlanner
{
public:
	FCampaignPlannerBase(UCampaignSubsystem& InOwner, int64 IntervalSeconds)
		: Owner(InOwner)
		, Gate(IntervalSeconds)
	{
	}

	void Reset(int64 NowSeconds) override
	{
		Gate.Reset(NowSeconds);
	}

protected:
	bool ShouldRun(const FCampaignTickContext& Ctx)
	{
		return Gate.Due(Ctx.NowSeconds);
	}

protected:
	UCampaignSubsystem& Owner;
	FCampaignGate Gate;
};
