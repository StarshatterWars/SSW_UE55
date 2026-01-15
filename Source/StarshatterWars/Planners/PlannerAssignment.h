// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CampaignPlanner.h"
#include "CampaignSubsystem.h"

class FPlannerAssignment final : public FCampaignPlannerBase
{
public:
	FPlannerAssignment(UCampaignSubsystem& InOwner, int64 IntervalSeconds)
		: FCampaignPlannerBase(InOwner, IntervalSeconds)
	{
	}

	const TCHAR* GetName() const override
	{
		return TEXT("Assignment");
	}

	void TickCampaign(const FCampaignTickContext& Ctx) override
	{
		if (!ShouldRun(Ctx))
			return;

		// TODO:
		// - Generate / refresh combat assignments
		// - Assign groups to zones/tasks
		UE_LOG(LogTemp, Log, TEXT("[Campaign][Assignment] t=%lld"), Ctx.NowSeconds);
	}
};

