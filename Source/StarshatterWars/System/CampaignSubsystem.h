#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CampaignPlanner.h"
#include "CampaignSubsystem.generated.h"

// -------------------------
// Plain C++ mission offer
// -------------------------
struct FMissionOffer
{
	int32 OfferId = -1;
	FName MissionTemplateRow;
	int64 StartTimeSeconds = 0;
	FString SourceTag;
};

// C++-only notification for UI (Ops screen) or other subscribers
DECLARE_MULTICAST_DELEGATE(FOnCampaignMissionOffersChanged);

// -------------------------
// Campaign Subsystem
// -------------------------
UCLASS()
class STARSHATTERWARS_API UCampaignSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// -------------------------
	// Lifecycle
	// -------------------------
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// -------------------------
	// Control
	// -------------------------
	void StartCampaign(int32 CampaignId);
	void StopCampaign();

	bool IsRunning() const { return bRunning; }
	int32 GetActiveCampaignId() const { return ActiveCampaignId; }

	// -------------------------
	// Master clock entrypoint
	// -------------------------

	// Bound to FOnUniverseSecond (uint64 UniverseSeconds)
	void HandleUniverseSecond(uint64 UniverseSeconds);

	// -------------------------
	// Mission feed
	// -------------------------
	const TArray<FMissionOffer>& GetMissionOffers() const { return MissionOffers; }
	FOnCampaignMissionOffersChanged OnMissionOffersChanged;

	// Helpers used by planners (safe to call from pure C++)
	int32 AllocateOfferId();
	void AddMissionOffer(FMissionOffer Offer);
	void ClearMissionOffers();

private:
	// Build planner stubs and reset their gating state
	void BuildPlanners();
	void ResetPlanners(int64 NowSeconds);

	// Placeholder: later you will hydrate caches from DataTables here
	void HydrateFromDataTables();

private:
	// State
	bool bRunning = false;
	int32 ActiveCampaignId = -1;

	// Determinism (seed later from save)
	FRandomStream Rng;

	// Planner objects are pure C++ and owned here
	TArray<TUniquePtr<ICampaignPlanner>> Planners;

	// Mission offers output (read by Ops)
	TArray<FMissionOffer> MissionOffers;
	int32 NextOfferId = 1;

	// Last known time (handy for StartCampaign reset)
	int64 LastNowSeconds = 0;
};
