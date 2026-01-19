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

#include <type_traits>

namespace
{
	template <typename T, typename = void>
	struct THasMember_RowName : std::false_type {};
	template <typename T>
	struct THasMember_RowName<T, std::void_t<decltype(std::declval<const T&>().RowName)>> : std::true_type {};

	template <typename T, typename = void>
	struct THasMember_TemplateRow : std::false_type {};
	template <typename T>
	struct THasMember_TemplateRow<T, std::void_t<decltype(std::declval<const T&>().TemplateRow)>> : std::true_type {};

	template <typename T, typename = void>
	struct THasMember_MissionTemplateRow : std::false_type {};
	template <typename T>
	struct THasMember_MissionTemplateRow<T, std::void_t<decltype(std::declval<const T&>().MissionTemplateRow)>> : std::true_type {};

	template <typename T>
	static FName ExtractTemplateRowName(const T& Elem)
	{
		if constexpr (std::is_same<T, FName>::value)
			return Elem;
		else if constexpr (THasMember_TemplateRow<T>::value)
			return Elem.TemplateRow;
		else if constexpr (THasMember_MissionTemplateRow<T>::value)
			return Elem.MissionTemplateRow;
		else if constexpr (THasMember_RowName<T>::value)
			return Elem.RowName;
		else
			return NAME_None;
	}
}

int32 UCampaignSubsystem::GetMaxMissionOffers() const
{
	// Classic Starshatter fighter cap.
	// Later: make data-driven (player asset type, difficulty, campaign, etc.)
	return 5;
}

bool UCampaignSubsystem::TryPickRandomMissionTemplateRow(FName& OutTemplateRow) const
{
	OutTemplateRow = NAME_None;

	if (!bHasActiveCampaign)
		return false;

	const auto& List = ActiveCampaignConfig.TemplateList;
	if (List.Num() <= 0)
		return false;

	const int32 StartIndex = Rng.RandRange(0, List.Num() - 1);

	for (int32 i = 0; i < List.Num(); ++i)
	{
		const int32 Idx = (StartIndex + i) % List.Num();
		const FName Row = ExtractTemplateRowName(List[Idx]);

		if (!Row.IsNone())
		{
			OutTemplateRow = Row;
			return true;
		}
	}

	return false;
}

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
	// - Do notxtouch DTs inside planners
	UE_LOG(LogTemp, Log, TEXT("[CampaignSubsystem] HydrateFromDataTables (stub)"));
}

void UCampaignSubsystem::StartCampaign(int32 CampaignId)
{
	ActiveCampaignId = CampaignId;
	bRunning = true;

	Rng.Initialize(FMath::Rand());

	NextOfferId = 1;
	ClearMissionOffers();

	bHasActiveCampaign = false;
	if (!LoadActiveCampaignRowByIndex(CampaignId))
	{
		UE_LOG(LogTemp, Error, TEXT("[CampaignSubsystem] StartCampaign failed (no campaign row)"));
		bRunning = false;
		ActiveCampaignId = -1;
		return;
	}

	ResetPlanners(LastNowSeconds);

	// Starshatter rule: Campaign 1 is training; 2+ is dynamic.
	// Data-driven fallback: if TemplateList exists, treat as dynamic.
	const bool bHasTemplates = ActiveCampaignConfig.TemplateList.Num() > 0;

	if (CampaignId == 1 && !bHasTemplates)
	{
		BuildTrainingOffers();  // Ops can display immediately
	}
	else
	{
		// Dynamic: let PlannerMission generate offers on tick
		UE_LOG(LogTemp, Log, TEXT("[CampaignSubsystem] Dynamic campaign: offers will be generated by planners."));
	}

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
	if (!bRunning)
		return;

	uint64 DeltaSeconds = 1;

	if (!bHasLastUniverseSeconds)
	{
		bHasLastUniverseSeconds = true;
		LastUniverseSeconds = UniverseSeconds;
		DeltaSeconds = 1;
	}
	else
	{
		// Handle missed ticks gracefully:
		DeltaSeconds = (UniverseSeconds > LastUniverseSeconds)
			? (UniverseSeconds - LastUniverseSeconds)
			: 0;

		LastUniverseSeconds = UniverseSeconds;
	}

	FCampaignTickContext Ctx;
	Ctx.NowSeconds = (int64)UniverseSeconds;
	Ctx.DeltaSeconds = (int64)DeltaSeconds;
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

bool UCampaignSubsystem::LoadActiveCampaignRowByIndex(int32 CampaignIndex)
{
	if (!CampaignDT)
	{
		UE_LOG(LogTemp, Error, TEXT("[CampaignSubsystem] CampaignDT is null. Call SetCampaignDataTable() first."));
		return false;
	}

	// DataTables are keyed by RowName, but your FS_Campaign has Index.
	// We therefore scan rows once per campaign start.
	TArray<FS_Campaign*> Rows;
	CampaignDT->GetAllRows(TEXT("LoadActiveCampaignRowByIndex"), Rows);

	for (const FS_Campaign* Row : Rows)
	{
		if (Row && Row->Index == CampaignIndex)
		{
			ActiveCampaignConfig = *Row;   // copy snapshot
			bHasActiveCampaign = true;

			UE_LOG(LogTemp, Log, TEXT("[CampaignSubsystem] Loaded campaign '%s' Index=%d  Missions=%d  Templates=%d  Actions=%d"),
				*ActiveCampaignConfig.Name,
				ActiveCampaignConfig.Index,
				ActiveCampaignConfig.MissionList.Num(),
				ActiveCampaignConfig.TemplateList.Num(),
				ActiveCampaignConfig.Action.Num());

			return true;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("[CampaignSubsystem] No campaign row found with Index=%d"), CampaignIndex);
	return false;
}

void UCampaignSubsystem::BuildTrainingOffers()
{
	ClearMissionOffers();

	// Campaign 1 training missions are the “static missions.def equivalent”
	for (const FS_CampaignMissionList& M : ActiveCampaignConfig.MissionList)
	{
		if (!M.Available || M.Complete)
			continue;

		FMissionOffer Offer;
		Offer.OfferId = AllocateOfferId();

		// For training missions, this is notxa template row; store the mission name/id as a tag.
		Offer.MissionTemplateRow = NAME_None;
		Offer.StartTimeSeconds = LastNowSeconds;
		Offer.SourceTag = FString::Printf(TEXT("TRAINING:%d:%s"), M.Id, *M.Name);

		AddMissionOffer(MoveTemp(Offer));
	}

	UE_LOG(LogTemp, Log, TEXT("[CampaignSubsystem] Training offers built: %d"), MissionOffers.Num());
}

static bool IsWithinWindow(const int64 Now, int32 StartAfter, int32 StartBefore)
{
	if (StartAfter > 0 && Now < (int64)StartAfter) return false;
	if (StartBefore > 0 && Now > (int64)StartBefore) return false;
	return true;
}

static bool RankOk(int32 PlayerRank, int32 MinRank, int32 MaxRank)
{
	if (MinRank > 0 && PlayerRank < MinRank) return false;
	if (MaxRank > 0 && PlayerRank > MaxRank) return false;
	return true;
}

bool UCampaignSubsystem::TryPickEligibleMissionTemplate(
	const FCampaignTickContext& Ctx,
	FName& OutTemplateRow,
	FString& OutDebug
) const
{
	OutTemplateRow = NAME_None;
	OutDebug.Empty();

	if (!bHasActiveCampaign)
		return false;

	const TArray<FS_CampaignTemplateList>& List = ActiveCampaignConfig.TemplateList;
	if (List.Num() <= 0)
		return false;

	// If you later wire player rank/iff properly, these become real.
	const int32 LocalPlayerRank = Ctx.PlayerRank;

	// We will gather eligible indices then pick randomly among them.
	TArray<int32> Eligible;
	Eligible.Reserve(List.Num());

	for (int32 i = 0; i < List.Num(); ++i)
	{
		const FS_CampaignTemplateList& T = List[i];

		// Must have something to execute
		if (T.Script.IsEmpty())
			continue;

		// Time window gate
		if (!IsWithinWindow(Ctx.NowSeconds, T.StartAfter, T.StartBefore))
			continue;

		// Rank gate
		if (!RankOk(LocalPlayerRank, T.MinRank, T.MaxRank))
			continue;

		// ExecOnce gate
		if (T.ExecOnce != 0 && ExecutedTemplateIds.Contains(T.Id))
			continue;

		// Action gating (optional, but struct supports it)
		// If ActionId is set, require ActionStatus match, if you’re tracking it.
		if (T.ActionId != 0)
		{
			const int32* Status = ActionStatusById.Find(T.ActionId);

			// If we have no status recorded, treat as “notxready” when ActionStatus is non-zero.
			if (T.ActionStatus != 0)
			{
				if (!Status || *Status != T.ActionStatus)
					continue;
			}
		}

		Eligible.Add(i);
	}

	if (Eligible.Num() <= 0)
		return false;

	// Randomly choose an eligible template
	const int32 PickIdx = (Ctx.Rng)
		? Eligible[Ctx.Rng->RandRange(0, Eligible.Num() - 1)]
		: Eligible[0];

	const FS_CampaignTemplateList& Pick = List[PickIdx];

	OutTemplateRow = FName(*Pick.Script);
	OutDebug = FString::Printf(
		TEXT("TID:%d NAME:%s SCRIPT:%s REGION:%s MT:%d GT:%d"),
		Pick.Id, *Pick.Name, *Pick.Script, *Pick.Region, Pick.MissionType, Pick.GroupType
	);

	return !OutTemplateRow.IsNone();
}

void UCampaignSubsystem::MarkTemplateExecutedIfOnce(const FS_CampaignTemplateList& T)
{
	if (T.ExecOnce != 0)
	{
		ExecutedTemplateIds.Add(T.Id);
	}
}

void UCampaignSubsystem::TickCampaignSeconds(uint64 UniverseSeconds)
{
	HandleUniverseSecond(UniverseSeconds);
}
static bool IsWithinWindowSeconds(int64 Now, int32 StartAfter, int32 StartBefore)
{
	if (StartAfter > 0 && Now < (int64)StartAfter) return false;
	if (StartBefore > 0 && Now > (int64)StartBefore) return false;
	return true;
}

bool UCampaignSubsystem::TryBuildOfferFromCampaignAction(const FCampaignTickContext& Ctx, FMissionOffer& OutOffer)
{
	OutOffer = FMissionOffer{};

	if (!bHasActiveCampaign)
		return false;

	if (ActiveCampaignConfig.Action.Num() <= 0)
		return false;

	// Scan for eligible MISSION_TEMPLATE actions.
	for (const FS_CampaignAction& A : ActiveCampaignConfig.Action)
	{
		if (!A.Type.Equals(TEXT("MISSION_TEMPLATE"), ESearchCase::IgnoreCase))
			continue;

		// Time window gate
		if (!IsWithinWindowSeconds(Ctx.NowSeconds, A.StartAfter, A.StartBefore))
			continue;

		// IFF gate (0 means wildcard/unrestricted)
		if (A.Iff != 0 && A.Iff != CachedPlayerIff)
			continue;

		// Rank gate (rank notxwired yet => CachedPlayerRank == 0, which is safe)
		if (!RankOk(CachedPlayerRank, A.MinRank, A.MaxRank))
			continue;

		// Cooldown: use Delay if present; else Starshatter default 7200 sec
		const int64 Cooldown = (A.Delay > 0) ? (int64)A.Delay : 7200;

		if (const int64* LastExec = ActionLastExecSeconds.Find(A.Id))
		{
			if (*LastExec > 0 && (Ctx.NowSeconds - *LastExec) < Cooldown)
				continue;
		}

		// Probability gate (0 = always)
		if (A.Probability > 0 && Ctx.Rng)
		{
			const int32 Roll = Ctx.Rng->RandRange(1, 100);
			if (Roll > A.Probability)
				continue;
		}

		// Determine mission template script name from action.
		// Prefer Source (common), then Scene.
		FName TemplateRow = NAME_None;
		if (!A.Source.IsEmpty())
			TemplateRow = FName(*A.Source);
		else if (!A.Scene.IsEmpty())
			TemplateRow = FName(*A.Scene);

		// If notxspecified, fall back to template list selection (but do notxrecurse)
		if (TemplateRow.IsNone())
			continue;

		OutOffer.OfferId = AllocateOfferId();
		OutOffer.MissionTemplateRow = TemplateRow;
		OutOffer.SourceTag = FString::Printf(
			TEXT("ACTION:%d SUB:%d IFF:%d SYS:%s REG:%s TITLE:%s"),
			A.Id, A.Subtype, A.Iff, *A.System, *A.Region, *A.Title
		);

		ActionLastExecSeconds.Add(A.Id, Ctx.NowSeconds);

		UE_LOG(LogTemp, Log, TEXT("[Campaign][Mission] Action->Offer actionId=%d template=%s"),
			A.Id, *TemplateRow.ToString());

		return true;
	}

	return false;
}

bool UCampaignSubsystem::TryBuildOfferFromTemplateList(const FCampaignTickContext& Ctx, FMissionOffer& OutOffer)
{
	OutOffer = FMissionOffer{};

	if (!bHasActiveCampaign)
		return false;

	const TArray<FS_CampaignTemplateList>& List = ActiveCampaignConfig.TemplateList;
	if (List.Num() <= 0)
		return false;

	// Player rank notxwired; treat as unknown.
	// This will naturally exclude templates that require MinRank > 0.
	const int32 LocalRank = CachedPlayerRank;

	TArray<int32> Eligible;
	Eligible.Reserve(List.Num());

	for (int32 i = 0; i < List.Num(); ++i)
	{
		const FS_CampaignTemplateList& T = List[i];

		if (T.Script.IsEmpty())
			continue;

		// Time window gate
		if (!IsWithinWindowSeconds(Ctx.NowSeconds, T.StartAfter, T.StartBefore))
			continue;

		// Rank gate
		if (!RankOk(LocalRank, T.MinRank, T.MaxRank))
			continue;

		// ExecOnce gate
		if (T.ExecOnce != 0 && ExecutedTemplateIds.Contains(T.Id))
			continue;

		// Optional action gating:
		// If Template references an ActionId + ActionStatus, require the action status match if you track it later.
		// For now, do notxblock on this since ActionStatus storage is notxwired.

		Eligible.Add(i);
	}

	if (Eligible.Num() <= 0)
		return false;

	const int32 PickIdx = (Ctx.Rng) ? Eligible[Ctx.Rng->RandRange(0, Eligible.Num() - 1)] : Eligible[0];
	const FS_CampaignTemplateList& Pick = List[PickIdx];

	const FName TemplateRow = FName(*Pick.Script);
	if (TemplateRow.IsNone())
		return false;

	OutOffer.OfferId = AllocateOfferId();
	OutOffer.MissionTemplateRow = TemplateRow;
	OutOffer.SourceTag = FString::Printf(
		TEXT("TEMPLATE:%d NAME:%s REGION:%s MT:%d GT:%d"),
		Pick.Id, *Pick.Name, *Pick.Region, Pick.MissionType, Pick.GroupType
	);

	if (Pick.ExecOnce != 0)
	{
		ExecutedTemplateIds.Add(Pick.Id);
	}

	UE_LOG(LogTemp, Log, TEXT("[Campaign][Mission] Template->Offer templateId=%d script=%s"),
		Pick.Id, *Pick.Script);

	return true;
}








