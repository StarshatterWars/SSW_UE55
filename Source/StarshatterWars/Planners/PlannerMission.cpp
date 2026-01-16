// Project nGenEx
// Fractal Dev Games
// Copyright (C) 2024. All Rights Reserved.
// 
// SUBSYSTEM:    SSW
// FILE:         PlannerMission.cpp
// AUTHOR:       Carlos Bott
// 

#include "PlannerMission.h"

namespace
{
	static int64 RoundUpToInterval(int64 Value, int64 Interval)
	{
		if (Interval <= 0)
			return Value;

		const int64 R = Value % Interval;
		return (R == 0) ? Value : (Value + (Interval - R));
	}

	static int64 SelectStartTimeSeconds(const TArray<FMissionOffer>& Existing, int64 NowSeconds)
	{
		// Starshatter feel: schedule in 30-minute blocks, at least one block out.
		constexpr int64 Block = 1800; // 30 min
		int64 Base = NowSeconds + Block;

		if (Existing.Num() > 0)
		{
			Base = FMath::Max(Base, Existing.Last().StartTimeSeconds);
		}

		return RoundUpToInterval(Base + Block, Block);
	}
}

void FPlannerMission::TickCampaign(const FCampaignTickContext& Ctx)
{
	if (!ShouldRun(Ctx))
		return;

	const FS_Campaign* Campaign = Owner.GetActiveCampaign();
	if (!Campaign)
		return;

	// Training campaigns: offers built in StartCampaign() via BuildTrainingOffers().
	const bool bDynamic = (Campaign->TemplateList.Num() > 0) || (Campaign->Action.Num() > 0);
	if (!bDynamic)
		return;

	// Offer cap (classic Starshatter fighter behavior is 5; can later be group-type driven)
	const int32 MaxOffers = Owner.GetMaxMissionOffers();
	if (Owner.GetMissionOffers().Num() >= MaxOffers)
		return;

	// 1) Try campaign actions (scripted beats)
	FMissionOffer Offer;
	if (Owner.TryBuildOfferFromCampaignAction(Ctx, Offer))
	{
		Offer.StartTimeSeconds = SelectStartTimeSeconds(Owner.GetMissionOffers(), Ctx.NowSeconds);
		Owner.AddMissionOffer(MoveTemp(Offer));
		return;
	}

	// 2) Fallback: eligible template list
	if (Owner.TryBuildOfferFromTemplateList(Ctx, Offer))
	{
		Offer.StartTimeSeconds = SelectStartTimeSeconds(Owner.GetMissionOffers(), Ctx.NowSeconds);
		Owner.AddMissionOffer(MoveTemp(Offer));
		return;
	}

	// Nothing eligible this tick.
}
