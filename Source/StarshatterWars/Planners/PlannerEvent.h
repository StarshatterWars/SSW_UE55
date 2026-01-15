// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CampaignPlanner.h"
#include "CampaignSubsystem.h"

class FPlannerEvent final : public FCampaignPlannerBase
{
public:
	FPlannerEvent(UCampaignSubsystem& InOwner, int64 IntervalSeconds)
		: FCampaignPlannerBase(InOwner, IntervalSeconds)
	{
	}

	const TCHAR* GetName() const override
	{
		return TEXT("Event");
	}

	void TickCampaign(const FCampaignTickContext& Ctx) override
	{
		if (!ShouldRun(Ctx))
			return;

		// TODO:
		// - Background combat resolution
		// - Random campaign events
		UE_LOG(LogTemp, Log, TEXT("[Campaign][Event] t=%lld"), Ctx.NowSeconds);
	}
};
