// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CampaignPlanner.h"
#include "../System/CampaignSubsystem.h"

class FPlannerMission final : public FCampaignPlannerBase
{
public:
	FPlannerMission(UCampaignSubsystem& InOwner, int64 IntervalSeconds)
		: FCampaignPlannerBase(InOwner, IntervalSeconds)
	{
	}

	const TCHAR* GetName() const override
	{
		return TEXT("Mission");
	}

	void TickCampaign(const FCampaignTickContext& Ctx) override
	{
		if (!ShouldRun(Ctx))
			return;

		// TODO:
		// - Check mission offer count
		// - Evaluate campaign actions / assignments
		// - Select mission templates
		// - Create FMissionOffer entries
		UE_LOG(LogTemp, Log, TEXT("[Campaign][Mission] t=%lld"), Ctx.NowSeconds);
	}
};
