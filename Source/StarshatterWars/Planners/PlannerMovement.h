// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CampaignPlanner.h"
#include "../System/CampaignSubsystem.h"

class FPlannerMovement final : public FCampaignPlannerBase
{
public:
	FPlannerMovement(UCampaignSubsystem& InOwner, int64 IntervalSeconds)
		: FCampaignPlannerBase(InOwner, IntervalSeconds)
	{
	}

	const TCHAR* GetName() const override
	{
		return TEXT("Movement");
	}

	void TickCampaign(const FCampaignTickContext& Ctx) override
	{
		if (!ShouldRun(Ctx))
			return;

		// TODO:
		// - Advance group movement along routes
		// - Update region/system occupancy
		UE_LOG(LogTemp, Log, TEXT("[Campaign][Movement] t=%lld"), Ctx.NowSeconds);
	}
};
