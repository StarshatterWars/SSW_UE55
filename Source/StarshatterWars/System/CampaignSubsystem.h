#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CampaignPlanner.h"
#include "GameStructs.h"   // contains FS_Campaign
#include "CampaignSubsystem.generated.h"



class UDataTable;
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

	// Inject DT pointer from your existing loader (clean transitional step)
	void SetCampaignDataTable(UDataTable* InCampaignDT) { CampaignDT = InCampaignDT; }

	// Read-only access for UI / planners
	const FS_Campaign* GetActiveCampaign() const { return bHasActiveCampaign ? &ActiveCampaignConfig : nullptr; }

	// Picks a mission template row from the active campaign's TemplateList.
	// Returns false if no usable template is available.
	bool TryPickRandomMissionTemplateRow(FName& OutTemplateRow) const;

	bool TryPickEligibleMissionTemplate(const FCampaignTickContext& Ctx, FName& OutTemplateRow, FString& OutDebug) const;
	
	void MarkTemplateExecutedIfOnce(const FS_CampaignTemplateList& T);
	
	// Cap how many offers can exist at once (fighters = 5 in classic Starshatter).
	int32 GetMaxMissionOffers() const;

	// Optional wrapper: lets other systems tick campaign without going through timer event
	void TickCampaignSeconds(uint64 UniverseSeconds);

	// Build a single offer candidate. Planner sets StartTimeSeconds and calls AddMissionOffer().
	bool TryBuildOfferFromCampaignAction(const FCampaignTickContext& Ctx, FMissionOffer& OutOffer);
	bool TryBuildOfferFromTemplateList(const FCampaignTickContext& Ctx, FMissionOffer& OutOffer);

private:
	// Action status cache (if you want ActionStatus gating to be persistent)
	TMap<int32, int32> ActionStatusById;
	
	// Build planner stubs and reset their gating state
	void BuildPlanners();
	void ResetPlanners(int64 NowSeconds);

	// Placeholder: later you will hydrate caches from DataTables here
	void HydrateFromDataTables();

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

	uint64 LastUniverseSeconds = 0;
	bool   bHasLastUniverseSeconds = false;

	// Active campaign config snapshot (copied from DT row)
	FS_Campaign ActiveCampaignConfig;
	bool bHasActiveCampaign = false;

	// Temporary: pointer to campaign DT (until you move GameData later)
	UDataTable* CampaignDT = nullptr;

	bool LoadActiveCampaignRowByIndex(int32 CampaignIndex);
	void BuildTrainingOffers();     // MissionList -> MissionOffers

	// Action/template gating state (subsystem-owned, planners remain stateless)
	TMap<int32, int64> ActionLastExecSeconds; // actionId -> last exec time (seconds)
	TSet<int32> ExecutedTemplateIds;          // templateId executed when ExecOnce != 0

	// notxwired yet; keep at defaults (0 means "unknown/unrestricted" for gating)
	int32 CachedPlayerRank = 0;
	int32 CachedPlayerIff = 0;
};

