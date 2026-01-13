/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         GameDataLoader.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Loader and Parser class for initial generation of Master Game Data Tables
	Will not be used after Dable Table is Generated.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Foundation/DataLoader.h"
#include "../Foundation/ParseUtil.h"
#include "../Foundation/Random.h"
#include "../Foundation/FormatUtil.h"
#include "../Foundation/Text.h"
#include "../Foundation/Term.h"
#include "../Foundation/GameLoader.h"

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/DataTable.h"
#include "HAL/FileManagerGeneric.h"
#include "GameStructs.h"

#include "../System/SSWGameInstance.h"
#include "GameDataLoader.generated.h"

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

UCLASS()
class STARSHATTERWARS_API AGameDataLoader : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGameDataLoader();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void LoadCampaignData(const char* FileName, bool full = false);
	void LoadZones(FString Path);
	void LoadMissionList(FString Path);
	void LoadTemplateList(FString Path);
	void LoadMission(FString Name);
	void LoadTemplateMission(FString Name);
	void LoadScriptedMission(FString Name);
	void ParseMission(const char* filename);
	void ParseNavpoint(TermStruct* val, const char* fn);
	void ParseObjective(TermStruct* val, const char* fn);
	void ParseInstruction(TermStruct* val, const char* fn);
	void ParseShip(TermStruct* val, const char* fn);
	void ParseLoadout(TermStruct* val, const char* fn);
	void ParseEvent(TermStruct* val, const char* fn);
	void ParseElement(TermStruct* val, const char* fn);
	void ParseScriptedTemplate(const char* fn);
	void ParseMissionTemplate(const char* fname);
	void ParseAlias(TermStruct* val, const char* fn);
	void ParseRLoc(TermStruct* val, const char* fn);
	void ParseCallsign(TermStruct* val, const char* fn);

	void ParseOptional(TermStruct* val, const char* fn);
	void LoadGalaxyMap();
	void ParseStar(TermStruct* val, const char* fn);
	void ParsePlanet(TermStruct* val, const char* fn);
	void ParseMoon(TermStruct* val, const char* fn);

	void ParseRegion(TermStruct* val, const char* fn);

	void ParseMoonMap(TermStruct* val, const char* fn);
	void ParseStarMap(TermStruct* val, const char* fn);
	void ParsePlanetMap(TermStruct* val, const char* fn);
	void ParseTerrain(TermStruct* val, const char* fn);

	void LoadStarsystems();

	void ParseStarSystem(const char* FileName);

	void InitializeCombatRoster();
	void LoadShipDesigns();
	void LoadSystemDesignsFromDT();
	void LoadSystemDesigns();

	void LoadOrderOfBattle(const char* filename, int team);

	EEMPIRE_NAME GetEmpireName(int32 emp);
	CombatGroup* CloneOver(CombatGroup* force, CombatGroup* clone, CombatGroup* group);
	void Unload();
	void SetStatus(int s);
	double Stardate();
	void Clear();
	Combatant* GetCombatant(const char* cname);

	void ParseCombatUnit();

	void LoadShipDesign(const char* fn);
	
	// Ship Components
	void ParsePower(TermStruct* val, const char* fn);
	void ParseDrive(TermStruct* val, const char* fn);

	void LoadSystemDesign(const char* fn);

	int ClassForName(const char* name);

	const char* ClassName(int type);

	FString GetOrdinal(int id);
	FString GetNameFromType(ECOMBATGROUP_TYPE name);

	FS_LayoutDef LayoutDef;
	
	Text GetContentBundleText(const char* key)   const;
	void GetSSWInstance();

	void InitializeCampaignData();
	USSWGameInstance* SSWInstance;

	void LoadContentBundle();
	void LoadForms();
	void ParseCtrlDef(TermStruct* val, const char* fn);
	void ParseLayoutDef(TermStruct* val, const char* fn);
	void LoadForm(const char* fname);
	void LoadAwardTables();
	bool IsContentBundleLoaded() const { return !ContentValues.IsEmpty(); }

	Text              ContentName;
	Dictionary<Text>  ContentValues;
	// attributes:
	int                  campaign_id;
	int                  status;
	int					 Index;
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

	

	//Bitmap               image[NUM_IMAGES];

	bool                 scripted;
	bool                 available;
	bool                 sequential;
	bool                 loaded_from_savegame;

	List<Combatant>      combatants;
	List<AStarSystem>    systems;
	List<CombatZone>     zones;
	List<CampaignPlan>   planners;
	List<MissionInfo>    missions;
	List<TemplateList>   templates;
	List<CombatAction>   actions;
	List<CombatEvent>    events;
	CombatGroup*		 player_group;
	CombatUnit*			 player_unit;

	int                  mission_id;
	Mission*			 mission;
	Mission*			 net_mission;

	double               time;
	double               loadTime;
	double               startTime;
	double               updateTime;
	int                  lockout;

protected:
	
	int ActionSize;
	int CombatantSize;
	int GroupSize;
	
	Text  GroupType;
	int   GroupId;

	Text CombatantName;
	Text CombatantType;
	int CombatantId;
	
	FS_Combatant NewCombatUnit;
	FS_CombatantGroup NewGroupUnit;

	FS_Campaign CampaignData;
	FS_Galaxy GalaxyData;
	FS_StarSystem StarSystemData;
	FS_ShipDesign ShipDesignData;
	FS_SystemDesign SystemDesignData;
	FS_AwardInfo AwardData;
	FS_CombatGroupUnit CombatGroupUnit;
	FS_CombatGroup CombatGroupData;
	FS_OOBForce ForceData;
	
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
	TArray<FS_SystemDesign> NewSystemArray;
	TArray<FS_ComponentDesign> NewComponentArray;
	TArray<FS_SystemDesign*> SystemDesignTable;

	TArray<FS_ShipPower> NewShipPowerArray;
	
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
	
	class UDataTable* StarSystemDataTable;
	class UDataTable* StarsDataTable;
	class UDataTable* PlanetsDataTable;
	class UDataTable* MoonsDataTable;
	class UDataTable* RegionsDataTable;
	class UDataTable* ShipDesignDataTable;
	class UDataTable* SystemDesignDataTable;
	class UDataTable* FormDefDataTable;
	class UDataTable* AwardsDataTable;

	//static List<SystemDesign>  catalog;

	int   ActionId;
	Text  ActionType;
	int   ActionSubtype;
	int   OppType;
	int   ActionTeam;
	Text  ActionSource;
	Vec3  ActionLocation;
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

	int  Action = 0;
	Text ActionStatus;
	bool NotAction = false;

	Text Combatant1;
	Text Combatant2;

	int		comp = 0;
	int     score = 0;
	int     intel = 0;
	int     gtype = 0;
	int     gid = 0;

	FString CampaignPath;

	Text UnitName;
	Text UnitRegnum;
	Text UnitRegion;
	Text UnitClass;
	Text UnitDesign;
	Text UnitSkin;
	Vec3 UnitLoc;
	int  UnitCount;
	int  UnitDamage;
	int  UnitDead;
	int  UnitHeading;

	int      AwardId;
	int		 AwardGrant;

	Text     AwardType;
	Text     AwardName;
	Text     AwardAbrv;
	Text     AwardDesc;
	Text     AwardText;

	Text     DescSound;
	Text     GrantSound;

	Text	LargeImage;
	Text	SmallImage;

	int		RequiredAwards;
	int		Lottery;
	int     MinShipClass;
	int     MaxShipClass;
	int		GrantedShipClasses;

	int		TotalPoints;
	int		MissionPoints;
	int		TotalMissions;

	int		Kills;
	int		Lost;
	int		Collision;
	int		CampaignId;

	bool	CampaignComplete;
	bool	DynamicCampaign;
	bool	Ceremony;
};

