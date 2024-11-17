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
	void ParseMission(const char* filename);
	void ParseAction(TermStruct* val, const char* filename);
	CombatGroup* CloneOver(CombatGroup* force, CombatGroup* clone, CombatGroup* group);
	void Unload();
	void SetStatus(int s);
	double Stardate();
	void Clear();
	Combatant* GetCombatant(const char* cname);
	void LoadGalaxyData();

	void GetSSWInstance();

	void InitializeCampaignData();
	USSWGameInstance* SSWInstance;

	// attributes:
	int                  campaign_id;
	int                  status;
	char                 filename[64];
	Text                 path[64];
	Text                 name;
	Text                 description;
	Text                 situation;
	Text                 orders;

	

	//Bitmap               image[NUM_IMAGES];

	bool                 scripted;
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
	class UDataTable* CampaignDataTable;
	TArray<FS_CampaignAction> CampaignActionArray;
	TArray<FS_Combatant> CombatantArray;
	TArray<FS_CampaignZone> ZoneArray;
	TArray<FS_CampaignMissionList> MissionListArray;
	TArray<FS_CampaignTemplateList> TemplateListArray;
	TArray<FS_CampaignMission> MissionArray;

	FS_CampaignAction NewCampaignAction;

	int   ActionId;
	Text  ActionType;
	int  ActionSubtype;
	int   OppType;
	int   ActionTeam;
	Text  ActionSource;
	Vec3  ActionLocation;
	Text  ActionSystem;
	Text  ActionRegion;
	Text  ActionFile;
	Text  ActionImage;
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

	int  act = 0;
	Text Actionstatus;
	bool not_action = false;

	Text Combatant1;
	Text Combatant2;

	Text	comp = "";
	int     score = 0;
	Text    intel = "";
	int         gtype = 0;
	int         gid = 0;

	FString CampaignPath;
};
