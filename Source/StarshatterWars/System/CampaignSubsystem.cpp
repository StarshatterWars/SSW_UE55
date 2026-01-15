// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "CampaignSubsystem.h"

// Timer Subsystem
#include "TimerSubsystem.h"

// Planner headers
#include "CampaignPlanner.h"
#include "PlannerStrategic.h"
#include "PlannerAssignment.h"
#include "PlannerMovement.h"
#include "PlannerMission.h"
#include "PlannerEvent.h"

void UCampaignSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Subscribe to your master clock tick:
	if (UTimerSubsystem* Timer = GetGameInstance()->GetSubsystem<UTimerSubsystem>())
	{
		// Expected signature: (int64 NowSeconds, int64 DeltaSeconds)
		Timer->OnUniverseSecond.AddUObject(this, &UCampaignSubsystem::HandleUniverseSecond);
	}

	BuildPlanners();

	UE_LOG(LogTemp, Log, TEXT("[CampaignSubsystem] Initialized"));
}

void UCampaignSubsystem::Deinitialize()
{
	StopCampaign();

	if (UTimerSubsystem* Timer = GetGameInstance()->GetSubsystem<UTimerSubsystem>())
	{
		Timer->OnUniverseSecond.RemoveAll(this);
	}

	Planners.Empty();
	MissionOffers.Empty();

	UE_LOG(LogTemp, Log, TEXT("[CampaignSubsystem] Deinitialized"));
	Super::Deinitialize();
}

void UCampaignSubsystem::BuildPlanners()
{
	Planners.Empty();

	// Cadences (seconds) - tune later:
	// Strategic: 60s
	// Assignment: 300s
	// Movement: 7200s
	// Mission: 1s
	// Event: 300s
	Planners.Add(MakeUnique<FPlannerStrategic>(*this, 60));
	Planners.Add(MakeUnique<FPlannerAssignment>(*this, 300));
	Planners.Add(MakeUnique<FPlannerMovement>(*this, 7200));
	Planners.Add(MakeUnique<FPlannerMission>(*this, 1));
	Planners.Add(MakeUnique<FPlannerEvent>(*this, 300));

	UE_LOG(LogTemp, Log, TEXT("[CampaignSubsystem] Built %d planners"), Planners.Num());
}

void UCampaignSubsystem::ResetPlanners(int64 NowSeconds)
{
	for (TUniquePtr<ICampaignPlanner>& Planner : Planners)
	{
		Planner->Reset(NowSeconds);
	}
}

void UCampaignSubsystem::HydrateFromDataTables()
{
	// Stub for now:
	// - Later, load DT rows into plain C++ caches here
	// - Do NOT touch DTs inside planners
	UE_LOG(LogTemp, Log, TEXT("[CampaignSubsystem] HydrateFromDataTables (stub)"));
}

void UCampaignSubsystem::StartCampaign(int32 CampaignId)
{
	ActiveCampaignId = CampaignId;
	bRunning = true;

	// Later: seed from save for determinism
	Rng.Initialize(FMath::Rand());

	NextOfferId = 1;
	ClearMissionOffers();

	// Load/prepare static data caches (later)
	HydrateFromDataTables();

	// Reset gating; run immediately from current known time
	ResetPlanners(LastNowSeconds);

	UE_LOG(LogTemp, Log, TEXT("[CampaignSubsystem] StartCampaign id=%d"), ActiveCampaignId);
}

void UCampaignSubsystem::StopCampaign()
{
	if (!bRunning)
		return;

	UE_LOG(LogTemp, Log, TEXT("[CampaignSubsystem] StopCampaign id=%d"), ActiveCampaignId);

	bRunning = false;
	ActiveCampaignId = -1;
}

void UCampaignSubsystem::HandleUniverseSecond(uint64 UniverseSeconds)
{
	LastNowSeconds = static_cast<int64>(UniverseSeconds);

	if (!bRunning)
		return;

	FCampaignTickContext Ctx;
	Ctx.NowSeconds = LastNowSeconds;
	Ctx.DeltaSeconds = 1;      // because this delegate is “per second”
	Ctx.Rng = &Rng;

	for (TUniquePtr<ICampaignPlanner>& Planner : Planners)
	{
		Planner->TickCampaign(Ctx);
	}
}

int32 UCampaignSubsystem::AllocateOfferId()
{
	return NextOfferId++;
}

void UCampaignSubsystem::AddMissionOffer(FMissionOffer Offer)
{
	MissionOffers.Add(MoveTemp(Offer));
	OnMissionOffersChanged.Broadcast();
}

void UCampaignSubsystem::ClearMissionOffers()
{
	MissionOffers.Reset();
	OnMissionOffersChanged.Broadcast();
}





