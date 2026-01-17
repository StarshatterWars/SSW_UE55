#include "Campaign.h"

// ----------------------------------------------------------------------------
// Static catalog state (optional; you can relocate to subsystem later)
// ----------------------------------------------------------------------------
static TArray<Campaign*> GCampaigns;
static Campaign* GCurrentCampaign = nullptr;

// ----------------------------------------------------------------------------
// FMissionInfo
// ----------------------------------------------------------------------------
FMissionInfo::FMissionInfo() = default;

FMissionInfo::~FMissionInfo()
{
	// Stub: original deletes Mission*; in UE you may own this differently.
	// Do not delete here until ownership is defined.
	MissionPtr = nullptr;
}

bool FMissionInfo::IsAvailable(Campaign* CampaignObj, const FCampaignRuntimeContext& Ctx)
{
	if (!CampaignObj)
		return false;

	const double Now = CampaignObj->IsRunning()
		? CampaignObj->GetCampaignTimeSeconds()
		: 0.0;

	if (Now < (double)StartAfter)
		return false;

	if (Now > (double)StartBefore)
		return false;

	if (MinRank && Ctx.PlayerRank < MinRank)
		return false;

	if (MaxRank && Ctx.PlayerRank > MaxRank)
		return false;

	// Exec-once logic mirrors Starshatter exactly
	if (ExecOnce < 0)
		return false;

	if (ExecOnce > 0)
		ExecOnce = -1;

	return true;
}

// ----------------------------------------------------------------------------
// Campaign
// ----------------------------------------------------------------------------
Campaign::Campaign(int32 InId, const FString& InName)
	: CampaignId(InId)
	, CampaignName(InName)
{
	Load();
}

Campaign::Campaign(int32 InId, const FString& InName, const FString& InPath)
	: CampaignId(InId)
	, CampaignName(InName)
	, CampaignPath(InPath)
{
	Load();
}

Campaign::~Campaign()
{
	Unload();

	// Catalog ownership is external; do not delete other campaigns here.
}

void Campaign::InitializeCatalog()
{
	// Stub: load Campaigns/%02d/campaign.def discovery like Starshatter.
	// For now, do nothing.
}

void Campaign::CloseCatalog()
{
	for (Campaign* C : GCampaigns)
	{
		delete C;
	}
	GCampaigns.Reset();
	GCurrentCampaign = nullptr;
}

Campaign* Campaign::GetCampaign()
{
	return GCurrentCampaign;
}

Campaign* Campaign::SelectCampaign(const FString& InName)
{
	for (Campaign* C : GCampaigns)
	{
		if (C && C->Name().Equals(InName, ESearchCase::IgnoreCase))
		{
			GCurrentCampaign = C;
			return C;
		}
	}
	return nullptr;
}

Campaign* Campaign::CreateCustomCampaign(const FString& InName, const FString& InPath)
{
	// Stub: allocate a new id beyond last dynamic/custom.
	const int32 NewId = GetLastCampaignId() + 1;
	Campaign* C = new Campaign(NewId, InName, InPath);
	GCampaigns.Add(C);
	return C;
}

TArray<Campaign*>& Campaign::GetAllCampaigns()
{
	return GCampaigns;
}

int32 Campaign::GetLastCampaignId()
{
	int32 Result = 0;
	for (Campaign* C : GCampaigns)
	{
		if (C)
			Result = FMath::Max(Result, C->GetCampaignId());
	}
	return Result;
}

void Campaign::Clear()
{
	// Mirror: missions.destroy(); planners.destroy(); combatants.destroy(); events.destroy(); actions.destroy();
	Missions.Reset();
	Planners.Reset();
	Combatants.Reset();
	Events.Reset();
	Actions.Reset();

	PlayerGroup = nullptr;
	PlayerUnit = nullptr;

	UpdateTime = TimeSeconds;
}

void Campaign::Load()
{
	// Stub:
	// - parse zones.def, campaign.def, missions.def, templates.def
	// - hydrate into internal arrays
	//
	// For now: make it safe and deterministic.
	Unload();
}

void Campaign::Unload()
{
	SetStatus(ECampaignStatus::CAMPAIGN_INIT);

	StartTime = 0.0;
	LoadTime = 0.0;
	LockoutSeconds = 0;

	CurrentMission = nullptr;
	NetMission = nullptr;

	Clear();

	Zones.Reset();
	Systems.Reset();
}

void Campaign::Prep()
{
	// Stub:
	// - create combatants if dynamic and empty
	// - load scripted missions if scripted and actions empty
	// - CheckPlayerGroup()
	CheckPlayerGroup();
}

void Campaign::Start()
{
	Prep();
	CreatePlanners();
	SetStatus(ECampaignStatus::CAMPAIGN_ACTIVE);
	bRunning = true;
}

void Campaign::Stop()
{
	bRunning = false;
}

void Campaign::Tick(const FCampaignRuntimeContext& Ctx)
{
	if (!bRunning)
		return;

	// Starshatter: time = Stardate() - startTime;
	// Here: campaign time advances from injected delta
	const int32 SafeDelta = (Ctx.DeltaSeconds > 0) ? Ctx.DeltaSeconds : 0;
	TimeSeconds += (double)SafeDelta;

	if (CampaignStatus < ECampaignStatus::CAMPAIGN_ACTIVE)
		return;

	// Stub: later you will execute planners, actions, mission generation, autosave triggers, etc.
	// For now: keep the method present and safe.
}

void Campaign::LockoutEvents(int32 Seconds)
{
	LockoutSeconds = FMath::Max(0, Seconds);
}

void Campaign::LoadCampaign(void* /*Loader*/, bool /*bFull*/)
{
	// Stub
}

void Campaign::ParseGroup(void* /*Val*/, CombatGroup* /*Force*/, CombatGroup* /*Clone*/, const TCHAR* /*Filename*/)
{
	// Stub
}

void Campaign::ParseAction(void* /*Val*/, const TCHAR* /*Filename*/)
{
	// Stub
}

CombatGroup* Campaign::CloneOver(CombatGroup* /*Force*/, CombatGroup* /*Clone*/, CombatGroup* /*Group*/)
{
	// Stub
	return nullptr;
}

void Campaign::LoadMissionList(void* /*Loader*/)
{
	// Stub
}

void Campaign::LoadCustomMissions(void* /*Loader*/)
{
	// Stub
}

void Campaign::LoadTemplateList(void* /*Loader*/)
{
	// Stub
}

void Campaign::CreatePlanners()
{
	// Stub:
	// later:
	// Planners.Add(MakeUnique<CampaignPlanEvent>(*this)) etc.
	Planners.Reset();
}

int32 Campaign::GetPlayerIFF() const
{
	// Stub: if PlayerGroup exists, return its IFF. Default 1.
	return 1;
}

void Campaign::SetPlayerGroup(CombatGroup* InPlayerGroup)
{
	PlayerGroup = InPlayerGroup;
	PlayerUnit = nullptr;

	// Starshatter destroys mission list for dynamic campaigns on group change.
	if (IsDynamic())
	{
		Missions.Reset();
	}
}

void Campaign::SetPlayerUnit(CombatUnit* /*Unit*/)
{
	// Stub: assign PlayerUnit and PlayerGroup, reset missions if dynamic
	if (IsDynamic())
	{
		Missions.Reset();
	}
}

void Campaign::CheckPlayerGroup()
{
	// Stub: original picks default wing/destroyer squadron from combatants.
}

CombatZone* Campaign::GetZone(const FString& /*Region*/)
{
	// Stub
	return nullptr;
}

void* Campaign::GetSystem(const FString& /*SystemName*/)
{
	// Stub
	return nullptr;
}

Combatant* Campaign::GetCombatant(const FString& /*CombatantName*/)
{
	// Stub
	return nullptr;
}

Mission* Campaign::GetMission()
{
	return GetMission(MissionId);
}

Mission* Campaign::GetMission(int32 /*Id*/)
{
	// Stub: later load mission file, generate sitrep for dynamic, etc.
	return nullptr;
}

Mission* Campaign::GetMissionByFile(const FString& /*Filename*/)
{
	// Stub
	return nullptr;
}

FMissionInfo* Campaign::CreateNewMission()
{
	// Stub
	return nullptr;
}

void Campaign::DeleteMission(int32 /*Id*/)
{
	// Stub
}

FMissionInfo* Campaign::GetMissionInfo(int32 /*Id*/)
{
	// Stub
	return nullptr;
}

void Campaign::ReloadMission(int32 /*Id*/)
{
	// Stub
}

void Campaign::LoadNetMission(int32 /*Id*/, const FString& /*NetMissionScript*/)
{
	// Stub
}

CombatAction* Campaign::FindAction(int32 /*ActionId*/)
{
	// Stub
	return nullptr;
}

FMissionInfo* Campaign::FindMissionTemplate(int32 /*MissionType*/, CombatGroup* /*InPlayerGroup*/)
{
	// Stub
	return nullptr;
}

FTemplateList* Campaign::GetTemplateList(int32 /*MissionType*/, int32 /*GroupType*/)
{
	// Stub
	return nullptr;
}

void Campaign::SetMissionId(int32 Id)
{
	if (Id > 0)
		MissionId = Id;
}

double Campaign::Stardate(const FCampaignRuntimeContext& Ctx) const
{
	// Map "stardate" to your universe time for now:
	// Starshatter used StarSystem::Stardate() double seconds.
	return (double)Ctx.NowSeconds;
}

void Campaign::StartMission()
{
	// Stub
}

void Campaign::RollbackMission()
{
	// Stub
}

bool Campaign::InCutscene() const
{
	// Stub: return false until integrated with your cutscene system
	return false;
}

bool Campaign::IsDynamic() const
{
	// Starshatter: campaign_id >= DYNAMIC_CAMPAIGN && campaign_id < SINGLE_MISSIONS
	// You can keep the same rule later; for now: treat 2+ as dynamic by default.
	return CampaignId >= 2;
}

bool Campaign::IsTraining() const
{
	return CampaignId == 1;
}

CombatGroup* Campaign::FindGroup(int32 /*Iff*/, int32 /*Type*/, int32 /*Id*/)
{
	// Stub
	return nullptr;
}

CombatGroup* Campaign::FindGroup(int32 /*Iff*/, int32 /*Type*/, CombatGroup* /*NearGroup*/)
{
	// Stub
	return nullptr;
}

CombatGroup* Campaign::FindStrikeTarget(int32 /*Iff*/, CombatGroup* /*StrikeGroup*/)
{
	// Stub
	return nullptr;
}

void Campaign::CommitExpiredActions()
{
	// Stub: mark available actions complete
	UpdateTime = TimeSeconds;
}

int32 Campaign::GetPlayerTeamScore()
{
	// Stub
	return 0;
}

void Campaign::SetStatus(ECampaignStatus InStatus)
{
	CampaignStatus = InStatus;

	// Starshatter destroys mission list when campaign ends:
	if (CampaignStatus > ECampaignStatus::CAMPAIGN_ACTIVE)
	{
		Missions.Reset();
	}
}

int32 Campaign::GetAllCombatUnits(int32 /*Iff*/, TArray<CombatUnit*>& OutUnits)
{
	OutUnits.Reset();
	return 0;
}

double Campaign::GetCampaignTimeSeconds() const
{
	return TimeSeconds;
}

void Campaign::SelectDefaultPlayerGroup(CombatGroup* /*Group*/, int32 /*Type*/)
{
	// Stub
}

double Campaign::GetCurrentStardate()
{
	Campaign* C = Campaign::GetCampaign();
	return C ? C->GetCurrentStardate() : 0.0;
}

int64 Campaign::GetTime() const
{
	// Starshatter compares campaign time to StartAfter/StartBefore gates (ints in seconds).
	// Your TimeSeconds is the running elapsed campaign time (seconds since StartTime).
	return (int64)TimeSeconds;
}

CombatGroup* Campaign::GetPlayerGroup() const
{
	return PlayerGroup;
}

CombatUnit* Campaign::GetPlayerUnit() const
{
	return PlayerUnit;
}

const TArray<Combatant*>& Campaign::GetCombatants() const
{
	return Combatants;
}

const TArray<CombatAction*>& Campaign::GetActions() const
{
	return Actions;
}