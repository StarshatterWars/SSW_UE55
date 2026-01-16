// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "PlannerMission.h"

namespace
{
	static int64 RoundUpToInterval(int64 Value, int64 Interval)
	{
		if (Interval <= 0)
			return Value;

		const int64 Remainder = Value % Interval;
		if (Remainder == 0)
			return Value;

		return Value + (Interval - Remainder);
	}
}

void FPlannerMission::TickCampaign(const FCampaignTickContext& Ctx)
{
	if (!ShouldRun(Ctx))
		return;

	// Training campaigns build offers up-front in StartCampaign().
	const FS_Campaign* Campaign = Owner.GetActiveCampaign();
	if (!Campaign)
		return;

	// No templates = nothing dynamic to generate.
	if (Campaign->TemplateList.Num() == 0)
		return;

	// Cap offers (fighters = 5 classic behavior).
	const int32 MaxOffers = Owner.GetMaxMissionOffers();
	if (Owner.GetMissionOffers().Num() >= MaxOffers)
		return;

	// Pick a mission template row.
	FName TemplateRow = NAME_None;
	if (!Owner.TryPickRandomMissionTemplateRow(TemplateRow))
		return;

	// Start time selection (classic behavior: 30-minute blocks)
	constexpr int64 MissionDelay = 1800; // 30 minutes

	int64 BaseTime = Ctx.NowSeconds + MissionDelay;

	const TArray<FMissionOffer>& Existing = Owner.GetMissionOffers();
	if (Existing.Num() > 0)
	{
		BaseTime = FMath::Max(BaseTime, Existing.Last().StartTimeSeconds);
	}

	const int64 StartTime = RoundUpToInterval(BaseTime + MissionDelay, MissionDelay);

	FMissionOffer Offer;
	Offer.OfferId = Owner.AllocateOfferId();
	Offer.MissionTemplateRow = TemplateRow;
	Offer.StartTimeSeconds = StartTime;
	Offer.SourceTag = FString::Printf(TEXT("DYNAMIC:%s"), *TemplateRow.ToString());

	Owner.AddMissionOffer(MoveTemp(Offer));

	UE_LOG(LogTemp, Log, TEXT("[Campaign][Mission] Offer %d template=%s start=%lld (now=%lld)"),
		Owner.GetMissionOffers().Last().OfferId,
		*TemplateRow.ToString(),
		StartTime,
		Ctx.NowSeconds);
}


