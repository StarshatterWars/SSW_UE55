// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CampaignPlanner.h"
#include "../System/CampaignSubsystem.h"

class FPlannerStrategic final : public FCampaignPlannerBase
{
public:
	FPlannerStrategic(UCampaignSubsystem& InOwner, int64 IntervalSeconds)
		: FCampaignPlannerBase(InOwner, IntervalSeconds)
	{
	}

	const TCHAR* GetName() const override
	{
		return TEXT("Strategic");
	}

	void TickCampaign(const FCampaignTickContext& Ctx) override
	{
		if (!ShouldRun(Ctx))
			return;

		// TODO:
		// - Evaluate strategic priorities
		// - Update campaign-wide threat / control state
		UE_LOG(LogTemp, Log, TEXT("[Campaign][Strategic] t=%lld"), Ctx.NowSeconds);
	}
};
