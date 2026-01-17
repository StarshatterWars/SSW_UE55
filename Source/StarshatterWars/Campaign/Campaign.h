#pragma once

#include "CoreMinimal.h"

// Forward declarations (you will replace these with real UE-side equivalents later)
class Combatant;
class CombatAction;
class CombatEvent;
class CombatGroup;
class CombatRoster;
class CombatUnit;
class CombatZone;
class Galaxy;
class Mission;

// -----------------------------------------------------------------------------
// Constants (match original intent)
// -----------------------------------------------------------------------------
static constexpr int32 TIME_NEVER = 1000000000;          // ~1e9
static constexpr int32 ONE_DAY = 24 * 3600;

// -----------------------------------------------------------------------------
// Runtime context injected by UCampaignSubsystem (or other owner).
// This replaces external Host access.
// -----------------------------------------------------------------------------
struct FCampaignRuntimeContext
{
	int64 NowSeconds = 0;  // Universe seconds
	int32 DeltaSeconds = 1;  // Tick delta, may be > 1
	int32 PlayerRank = 0;
	int32 PlayerIFF = 1;
};

// -----------------------------------------------------------------------------
// Mirror of Starshatter MissionInfo (stub)
// -----------------------------------------------------------------------------
struct FMissionInfo
{
	FMissionInfo();
	~FMissionInfo();

	Mission* MissionPtr = nullptr;

	int32 Start = 0;
	int32 Type = 0;
	int32 Id = 0;

	int32 MinRank = 0;
	int32 MaxRank = 0;
	int32 ActionId = 0;
	int32 ActionStatus = 0;

	int32 ExecOnce = 0;
	int32 StartBefore = TIME_NEVER;
	int32 StartAfter = 0;

	FString Name;
	FString Description;
	FString System;
	FString Region;
	FString Script; // filename or template script name

	// Availability gate (stub logic implemented in cpp)
	bool IsAvailable(class Campaign* CampaignObj, const FCampaignRuntimeContext& Ctx);
};

// -----------------------------------------------------------------------------
// Template bucket (mission_type + group_type) (stub)
// -----------------------------------------------------------------------------
struct FTemplateList
{
	int32 MissionType = 0;
	int32 GroupType = 0;
	int32 Index = 0;

	TArray<TUniquePtr<FMissionInfo>> Missions;
};

// -----------------------------------------------------------------------------
// Campaign status mirror
// -----------------------------------------------------------------------------
enum class ECampaignStatus : uint8
{
	CAMPAIGN_INIT = 0,
	CAMPAIGN_ACTIVE,
	CAMPAIGN_SUCCESS,
	CAMPAIGN_FAILURE
};

// -----------------------------------------------------------------------------
// Campaign core class (plain C++ object; owned by subsystem)
// -----------------------------------------------------------------------------
class Campaign
{
public:
	// Construction mirrors original: id + name (+ optional path)
	Campaign(int32 InId, const FString& InName);
	Campaign(int32 InId, const FString& InName, const FString& InPath);
	~Campaign();

	// Catalog (optional; you can move this to subsystem later)
	static void InitializeCatalog(); // stub
	static void CloseCatalog();

	static Campaign* GetCampaign(); // current_campaign
	static Campaign* SelectCampaign(const FString& Name);
	static Campaign* CreateCustomCampaign(const FString& Name, const FString& Path);

	static TArray<Campaign*>& GetAllCampaigns();
	static int32 GetLastCampaignId();

	// ---------------------------------------------------------------------
	// Lifecycle
	// ---------------------------------------------------------------------
	void Clear();
	void Load();            // stub
	void Unload();          // stub
	void Prep();            // stub
	void Start();           // stub
	void Stop();            // stub

	// Replacement for Starshatter ExecFrame: subsystem calls Tick with Ctx.
	void Tick(const FCampaignRuntimeContext& Ctx);

	void LockoutEvents(int32 Seconds);

	// ---------------------------------------------------------------------
	// Data loading (stubs)
	// ---------------------------------------------------------------------
	void LoadCampaign(void* Loader, bool bFull = true);
	void ParseGroup(void* Val, CombatGroup* Force, CombatGroup* Clone, const TCHAR* Filename);
	void ParseAction(void* Val, const TCHAR* Filename);

	CombatGroup* CloneOver(CombatGroup* Force, CombatGroup* Clone, CombatGroup* Group);

	void LoadMissionList(void* Loader);
	void LoadCustomMissions(void* Loader);
	void LoadTemplateList(void* Loader);

	// ---------------------------------------------------------------------
	// Planners (stubs; later store TUniquePtr<ICampaignPlanner>)
	// ---------------------------------------------------------------------
	void CreatePlanners();

	// ---------------------------------------------------------------------
	// Player selection / validation (stubs)
	// ---------------------------------------------------------------------
	int32 GetPlayerIFF() const;
	void SetPlayerGroup(CombatGroup* InPlayerGroup);
	void SetPlayerUnit(CombatUnit* Unit);
	void CheckPlayerGroup();

	// ---------------------------------------------------------------------
	// Zones / Systems / Combatants (stubs)
	// ---------------------------------------------------------------------
	CombatZone* GetZone(const FString& Region);
	void* GetSystem(const FString& SystemName);
	Combatant* GetCombatant(const FString& CombatantName);

	// ---------------------------------------------------------------------
	// Missions (stubs)
	// ---------------------------------------------------------------------
	Mission* GetMission();
	Mission* GetMission(int32 Id);
	Mission* GetMissionByFile(const FString& Filename);

	FMissionInfo* CreateNewMission();
	void DeleteMission(int32 Id);

	FMissionInfo* GetMissionInfo(int32 Id);
	void ReloadMission(int32 Id);
	void LoadNetMission(int32 Id, const FString& NetMissionScript);

	// ---------------------------------------------------------------------
	// Actions / Templates (stubs)
	// ---------------------------------------------------------------------
	CombatAction* FindAction(int32 ActionId);
	FMissionInfo* FindMissionTemplate(int32 MissionType, CombatGroup* InPlayerGroup);
	FTemplateList* GetTemplateList(int32 MissionType, int32 GroupType);

	// ---------------------------------------------------------------------
	// Mission state control (stubs)
	// ---------------------------------------------------------------------
	void SetMissionId(int32 Id);

	// Stardate mapping:
	// In Starshatter this was StarSystem::Stardate().
	// In Unreal you can map this to your Universe clock or a base-time offset.
	double Stardate(const FCampaignRuntimeContext& Ctx) const;

	void StartMission();     // stub
	void RollbackMission();  // stub

	// ---------------------------------------------------------------------
	// Queries / flags
	// ---------------------------------------------------------------------
	bool InCutscene() const; // stub
	bool IsDynamic() const;
	bool IsTraining() const;
	bool IsScripted() const { return bScripted; }
	bool IsSequential() const { return bSequential; }

	// ---------------------------------------------------------------------
	// Group queries (stubs)
	// ---------------------------------------------------------------------
	CombatGroup* FindGroup(int32 Iff, int32 Type, int32 Id);
	CombatGroup* FindGroup(int32 Iff, int32 Type, CombatGroup* NearGroup);
	CombatGroup* FindStrikeTarget(int32 Iff, CombatGroup* StrikeGroup);

	// ---------------------------------------------------------------------
	// Action bookkeeping / scoring (stubs)
	// ---------------------------------------------------------------------
	void CommitExpiredActions();
	int32 GetPlayerTeamScore();
	void SetStatus(ECampaignStatus InStatus);

	// ---------------------------------------------------------------------
	// Unit aggregation (stub)
	// ---------------------------------------------------------------------
	int32 GetAllCombatUnits(int32 Iff, TArray<CombatUnit*>& OutUnits);

	// ---------------------------------------------------------------------
	// Accessors
	// ---------------------------------------------------------------------
	int32 GetCampaignId() const { return CampaignId; }
	const FString& Name() const { return CampaignName; }
	const FString& Path() const { return CampaignPath; }
	ECampaignStatus Status() const { return CampaignStatus; }
	bool IsRunning() const { return bRunning; }

	double GetCampaignTimeSeconds() const;

	static double GetCurrentStardate();
	int64 GetTime() const;   // mirrors original Campaign::GetTime()

	CombatGroup* GetPlayerGroup() const;   // mirror of Starshatter Campaign::GetPlayerGroup()
	CombatUnit* GetPlayerUnit() const;   // mirror of original Campaign::GetPlayerUnit()

	const TArray<Combatant*>& GetCombatants() const;
	const TArray<CombatAction*>& GetActions() const;

private:
	// helpers (stub)
	void SelectDefaultPlayerGroup(CombatGroup* Group, int32 Type);

private:
	// Identity
	int32   CampaignId = 0;
	FString CampaignName;
	FString CampaignPath;

	// Mission runtime
	int32   MissionId = -1;
	Mission* CurrentMission = nullptr;
	Mission* NetMission = nullptr;

	// Flags
	bool bScripted = false;
	bool bSequential = false;

	// Time: Starshatter used double seconds since startTime
	double TimeSeconds = 0.0;
	double StartTime = 0.0;
	double LoadTime = 0.0;

	// Running state
	bool bRunning = false;

	// Player context (stubs)
	CombatGroup* PlayerGroup = nullptr;
	CombatUnit* PlayerUnit = nullptr;

	// Campaign state
	ECampaignStatus CampaignStatus = ECampaignStatus::CAMPAIGN_INIT;
	int32 LockoutSeconds = 0;
	bool  bLoadedFromSaveGame = false;

	// Data containers (stubs; convert to UObjects/structs later)
	TArray<CombatAction*> Actions;
	TArray<CombatEvent*>  Events;
	TArray<TUniquePtr<FMissionInfo>> Missions;
	TArray<TUniquePtr<FTemplateList>> Templates;
	TArray<void*> Planners;     // later: TUniquePtr<ICampaignPlanner>
	TArray<CombatZone*> Zones;
	TArray<Combatant*> Combatants;

	// Systems list (StarSystem* in original) - stub as void*
	TArray<void*> Systems;

	// Strings (mirror)
	FString Description;
	FString Situation;
	TArray<FString> Orders;

	// Bookkeeping
	double UpdateTime = 0.0;
};
