/*=============================================================================
    Project:        Starshatter Wars
    Studio:         Fractal Dev Games
    Copyright:      (C) 2024-2026. All Rights Reserved.

    SUBSYSTEM:      StarshatterWars (Unreal Engine)
    FILE:           StarshatterGameDataSubsystem.h
    AUTHOR:         Carlos Bott

    OVERVIEW
    ========
    Global game data loader and registry owner.

    This subsystem replaces the legacy AGameDataLoader actor.
    It is responsible for loading, parsing, and caching all
    static and semi-static game data required at runtime.

    The subsystem has no world presence and does not tick.
    It exists purely as a service and data registry.

    RESPONSIBILITIES
    ================
    - Load and parse legacy data files (cfg, def, script, text)
    - Generate and populate Unreal DataTables
    - Build registries for:
        * Galaxy and star systems
        * Campaigns and order-of-battle data
        * Missions, templates, and scripted scenarios
        * Ship, system, and component designs
        * Awards, forms, and UI layout definitions
    - Provide read-only access to loaded data (DT-first)

    NON-GOALS
    =========
    - No Actor spawning
    - No Tick()
    - No Transform or spatial behavior
    - No UI or presentation logic
    - No player persistence
    - No runtime “active campaign” state

    RUNTIME SEPARATION
    ==================
    Player save + active session state belong in:
      - UStarshatterPlayerSubsystem
      - UStarshatterDataRuntimeSubsystem

    This subsystem is STATIC DATA ONLY.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

// Legacy parsing / registry headers (kept for now)
#include "DataLoader.h"
#include "ParseUtil.h"
#include "Random.h"
#include "FormatUtil.h"
#include "Text.h"
#include "Term.h"
#include "GameLoader.h"

// Engine / file helpers
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/DataTable.h"
#include "HAL/FileManagerGeneric.h"

// Project types
#include "GameStructs.h"
#include "GameStructs_System.h"

#include "StarshatterGameDataSubsystem.generated.h"

// ---------------------------------------------------------------------
// Forward declarations (legacy)
// ---------------------------------------------------------------------
class Campaign;
class CampaignPlan;
class Combatant;
class CombatAction;
class CombatEvent;
class CombatGroup;
class CombatUnit;
class CombatZone;
class DataLoader;
class Mission;
class MissionTemplate;
class TemplateList;
class MissionInfo;
class AStarSystem;
class SystemDesign;
class ComponentDesign;

// Cached GI (optional)
class USSWGameInstance;

DECLARE_LOG_CATEGORY_EXTERN(LogStarshatterGameData, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogStarshatterGameDataCampaign, Log, All);

// ---------------------------------------------------------------------
// UStarshatterGameDataSubsystem
// ---------------------------------------------------------------------
UCLASS()
class STARSHATTERWARS_API UStarshatterGameDataSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // =====================================================================
    // Subsystem lifecycle
    // =====================================================================
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    void GetSSWInstance();

    UDataTable* GetCampaignDataTable() const { return CampaignDataTable; }
    const TArray<FS_Campaign>& GetCampaignDataArray() const { return CampaignDataArray; }

    // =====================================================================
    // Primary entry point
    // =====================================================================
    // Called by GameInitSubsystem during EGameMode::INIT
    void LoadAll(bool bFull = false);

    // =====================================================================
    // Project path / utility
    // =====================================================================
    void SetProjectPath();
    FString GetProjectPath();

    bool GetRegionTypeFromString(const FString& InString, EOrbitalType& OutValue);

    // =====================================================================
    // Campaigns (static load + DT hydration)
    // =====================================================================
    void LoadCampaignData(const char* FileName, bool full = false);

    // DT hydration methods (static -> DT/arrays)
    void ReadCampaignData();
    void ReadCombatRosterData();

    // Static campaign lookup (DT-first)
    bool TryGetCampaignByRowName(FName RowName, FS_Campaign& OutCampaign) const;
    bool TryGetCampaignByIndex1Based(int32 CampaignIndex1Based, FS_Campaign& OutCampaign) const;

    // =====================================================================
    // Missions / templates
    // =====================================================================
    void LoadZones(
        const FString& Path,
        const FString& CampaignId);

    void LoadMissionList(FString Path);
    void LoadTemplateList(FString Path);

    void LoadMission(FString Name);
    void LoadTemplateMission(FString Name);
    void LoadScriptedMission(FString Name);

    // Mission parsing
    void ParseMission(const char* filename);
    void ParseNavpoint(TermStruct* val, const char* fn);
    void ParseObjective(TermStruct* val, const char* fn);
    void ParseInstruction(TermStruct* val, const char* fn);
    void ParseShip(TermStruct* val, const char* fn);
    void ParseMissionLoadout(TermStruct* val, const char* fn);
    void ParseEvent(TermStruct* val, const char* fn);
    void ParseElement(TermStruct* val, const char* fn);
    void ParseScriptedTemplate(const char* fn);
    void ParseMissionTemplate(const char* fname);
    void ParseAlias(TermStruct* val, const char* fn);
    void ParseRLoc(TermStruct* val, const char* fn);
    void ParseCallsign(TermStruct* val, const char* fn);
    void ParseOptional(TermStruct* val, const char* fn);

    // =====================================================================
    // Galaxy
    // =====================================================================
    void LoadGalaxyMap();
    void ParseStar(TermStruct* val, const char* fn);
    void ParsePlanet(TermStruct* val, const char* fn);
    void ParseMoon(TermStruct* val, const char* fn);
    void ParseRegion(TermStruct* val, const char* fn);
    void ParseMoonMap(TermStruct* val, const char* fn);
    void ParseStarMap(TermStruct* val, const char* fn);
    void ParsePlanetMap(TermStruct* val, const char* fn);
    void ParseTerrain(TermStruct* val, const char* fn);

    // =====================================================================
    // Star systems / designs / OOB
    // =====================================================================
    void LoadStarsystems();
    void ParseStarSystem(const char* FileName);

    void InitializeCampaignData();
    void InitializeCombatRoster();

    void LoadOrderOfBattle(const char* InFilename, int32 Team);

    void ParseCombatUnit();

    // OOB helpers
    CombatGroup* CloneOver(CombatGroup* force, CombatGroup* clone, CombatGroup* group);

    void CreateOrderOfBattleTable();
    void ExportDataToCSV(UDataTable* DataTable, const FString& FileName);

    // =====================================================================
    // Content bundles / forms / awards
    // =====================================================================
    Text GetContentBundleText(const char* key) const;

    void LoadContentBundle();
    void LoadAwardTables();

    UFUNCTION(BlueprintPure, Category = "Starshatter|Awards")
    UDataTable* GetRanksTable() const { return DT_Ranks; }

    UFUNCTION(BlueprintPure, Category = "Starshatter|Awards")
    UDataTable* GetMedalsTable() const { return DT_Medals; }

    // Optional: unified view
    UFUNCTION(BlueprintPure, Category = "Starshatter|Awards")
    UDataTable* GetAllAwardsTable() const { return DT_AwardsAll; }

    // =====================================================================
    // Lifetime / state
    // =====================================================================
    void Unload();
    void Clear();

    void SetCampaignStatus(ECampaignStatus s);
    double Stardate();

    Combatant* GetCombatant(const char* cname);

    bool IsContentBundleLoaded() const { return !ContentValues.IsEmpty(); }
    bool IsLoaded() const { return bLoaded; }

    // =====================================================================
    // Public read lists used by UI (static data)
    // =====================================================================
    UPROPERTY()
    TArray<FS_CampaignMissionList> MissionList;

    UPROPERTY()
    TArray<FS_OOBForce> ForceList;

private:
    // =====================================================================
    // Internal helpers
    // =====================================================================
    void CacheSSWInstance();

private:
    // =====================================================================
    // Internal state flags
    // =====================================================================
    bool bLoaded = false;

public:
    // =====================================================================
    // Legacy fields (kept intact; grouped for readability)
    // =====================================================================

    Text                 ContentName;
    Dictionary<Text>     ContentValues;

    // Campaign header / metadata scratch
    int                  campaign_id;
    ECampaignStatus      CampaignStatus;
    int                  CampaignIndex;
    FString              CurrentCampaignId;
    char                 filename[64];
    Text                 path[64];
    Text                 name;
    Text                 description;
    Text                 situation;
    Text                 system;
    Text                 region;
    Text                 start;
    Text                 MainImage;
    Text                 orders;

    bool                 scripted;
    bool                 available;
    bool                 sequential;
    bool                 loaded_from_savegame;

    // Legacy registries
    List<Combatant>      combatants;
    List<AStarSystem>    systems;
    List<CombatZone>     zones;
    List<CampaignPlan>   planners;
    List<MissionInfo>    missions;
    List<TemplateList>   templates;
    List<CombatAction>   actions;
    List<CombatEvent>    events;

    bool                 bClearTables;
    
    UPROPERTY(Transient)
    bool bAwardTablesLoaded = false;

protected:
    // =====================================================================
    // Parse scratch sizes / ids
    // =====================================================================
    int ActionSize;
    int CombatantSize;
    int GroupSize;

    Text  GroupType;
    int   GroupId;

    Text CombatantName;
    Text CombatantType;
    int CombatantId;

    // =====================================================================
    // DataTables (kept intact)
    // =====================================================================
    UDataTable* CampaignDataTable;
    UDataTable* CombatGroupDataTable;
    UDataTable* OrderOfBattleDataTable;
    UDataTable* CampaignOOBDataTable;
    UDataTable* GalaxyDataTable;

    // =====================================================================
    // DT row scratch + arrays (kept intact)
    // =====================================================================
    FS_Combatant        NewCombatUnit;
    FS_CombatantGroup   NewGroupUnit;

    TArray<FS_Campaign>    CampaignDataArray;
    TArray<FS_CombatGroup> CombatRosterData;
    TArray<FS_Galaxy>      GalaxyDataArray;
    TArray<TArray<uint8>> SystemDesignStringStorage;

    FS_Campaign        CampaignData;
    FS_Galaxy          GalaxyData;
    FS_StarSystem      StarSystemData;
    
    FShipDesign        ShipDesignData;

    FS_CombatGroupUnit CombatGroupUnit;
    FS_CombatGroup     CombatGroupData;
    FS_OOBForce        ForceData;

    // =====================================================================
    // Large working arrays (kept intact)
    // =====================================================================
    TArray<FS_OOBFleet> FleetArray;
    TArray<FS_CampaignAction> CampaignActionArray;
    TArray<FS_Combatant> CombatantArray;
    TArray<FS_CampaignZone> ZoneArray;
    TArray<FS_CampaignMissionList> MissionListArray;
    TArray<FS_CampaignTemplateList> TemplateListArray;
    TArray<FS_CampaignMission> MissionArray;
    TArray<FS_MissionElement> MissionElementArray;
    TArray<FS_MissionEvent> MissionEventArray;
    TArray<FS_MissionLoadout> MissionLoadoutArray;
    TArray<FS_MissionCallsign> MissionCallsignArray;
    TArray<FS_MissionOptional> MissionOptionalArray;
    TArray<FS_MissionAlias> MissionAliasArray;
    TArray<FS_MissionShip> MissionShipArray;
    TArray<FS_RLoc> MissionRLocArray;
    TArray<FS_CampaignReq> CampaignActionReqArray;
    TArray<FS_MissionInstruction> MissionInstructionArray;
    TArray<FS_MissionInstruction> MissionObjectiveArray;
    TArray<FS_MissionInstruction> MissionNavpointArray;

    TArray<FS_CombatGroupUnit> NewCombatUnitArray;

    TArray<FS_Star> StarDataArray;
    TArray<FS_Planet> PlanetDataArray;
    TArray<FS_Moon> MoonDataArray;
    TArray<FS_Region> RegionDataArray;

    TArray<FS_StarMap> StarMapArray;
    TArray<FS_PlanetMap> PlanetMapArray;
    TArray<FS_MoonMap> MoonMapArray;
    TArray<FS_RegionMap> RegionMapArray;

    TArray<FS_TemplateMission> TemplateMissionArray;
    TArray<FS_TemplateMission> ScriptedMissionArray;

    FS_CampaignAction NewCampaignAction;

    // DataTables (additional)
    UDataTable* StarSystemDataTable;
    UDataTable* StarsDataTable;
    UDataTable* PlanetsDataTable;
    UDataTable* MoonsDataTable;
    UDataTable* RegionsDataTable;
    UDataTable* ZonesDataTable;
    UDataTable* ShipDesignDataTable;
    UDataTable* SystemDesignDataTable;
    
    UDataTable* AwardsDataTable;
    UDataTable* RanksDataTable;
    UDataTable* MedalsDataTable;

    // CampaignAction parse scratch
    int      ActionId;
    Text     ActionType;
    int      ActionSubtype;
    int      OppType;
    int      ActionTeam;
    Text     ActionSource;
    FVector  ActionLocation;
    Text  ActionSystem;
    Text  ActionRegion;
    Text  ActionFile;
    Text  ActionImage;
    Text  ActionAudio;
    Text  ActionDate;
    Text  ActionScene;
    Text  ActionText;

    int   ActionCount;
    int   StartBefore;
    int   StartAfter;
    int   MinRank;
    int   MaxRank;
    int   Delay;
    int   Probability;

    Text  AssetType;
    int   AssetId;
    Text  TargetType;
    int   TargetId;
    int   TargetIff;

    Text  AssetKill;
    Text  TargetKill;

    Text ZoneRegion;
    Text ZoneSystem;

    int  Action;
    ECombatActionStatus ActionStatus;
    bool NotAction;

    Text Combatant1;
    Text Combatant2;

    int  comp;
    int  score;
    int  intel;
    int  gtype;
    int  gid;

    FString CampaignPath;

    // Unit scratch
    Text UnitName;
    Text UnitRegnum;
    Text UnitRegion;
    Text UnitClass;
    Text UnitDesign;
    Text UnitSkin;
    FVector  UnitLoc;
    int     UnitCount;
    int     UnitDamage;
    int     UnitDead;
    int     UnitHeading;
    
    float    CurrentShipScale = 1.0f;

    // Cached GI (kept)
    USSWGameInstance* SSWInstance = nullptr;

    // Paths
    FString ProjectPath;
    FString FilePath;

    UPROPERTY()
    TMap<FString, FWeaponDesignMeta> WeaponMetaByType;

    // ------------------------------------------------------------
    // Per-ship selection results (reset each time you start a ship)
    // ------------------------------------------------------------
    int32 CurrentShipDecoyWeaponIndex = INDEX_NONE;
    int32 CurrentShipProbeWeaponIndex = INDEX_NONE;
    FString CurrentShipDecoyWeaponType;
    FString CurrentShipProbeWeaponType;
    FString CurrentShipSourceFile;

    // NOTE: these should be constructed somewhere (Initialize or on-demand)
    UPROPERTY(Transient)
    TObjectPtr<UDataTable> DT_Ranks = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UDataTable> DT_Medals = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UDataTable> DT_AwardsAll = nullptr;

    void EnsureAwardTables();
};

static FSkinMtlCell ParseSkinMtlCell(TermStruct* Val, const char* Fn);
