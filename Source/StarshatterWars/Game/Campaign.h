/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         Campaign.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Campaign defines a strategic military scenario.
*/

#pragma once

#include "CoreMinimal.h"
#include "../Foundation/Text.h"
#include "../Foundation/Geometry.h"
#include "../Foundation/Color.h"
#include "../Foundation/Term.h"
#include "../Foundation/List.h"
#include "../Foundation/DataLoader.h"
#include "GameFramework/Actor.h"
#include "../System/SSWGameInstance.h"
#include "Campaign.generated.h"

// +--------------------------------------------------------------------+

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
class MissionInfo;
class TemplateList;
class MissionTemplate;
class AStarSystem;

// +--------------------------------------------------------------------+

UCLASS()
class STARSHATTERWARS_API ACampaign : public AActor
{
	GENERATED_BODY()
	
public:	
	
	enum CONSTANTS {
		TRAINING_CAMPAIGN = 1,
		DYNAMIC_CAMPAIGN,
		MOD_CAMPAIGN = 100,
		SINGLE_MISSIONS = 1000,
		MULTIPLAYER_MISSIONS,
		CUSTOM_MISSIONS,

		NUM_IMAGES = 6
	};

	enum STATUS {
		CAMPAIGN_INIT,
		CAMPAIGN_ACTIVE,
		CAMPAIGN_SUCCESS,
		CAMPAIGN_FAILED
	};
	
	// Sets default values for this actor's properties
	ACampaign();

	static const char* TYPENAME() { return "Campaign"; }
	
	int operator == (const ACampaign& s) const { return name == s.name; }
	int operator <  (const ACampaign& s) const { return campaign_id < s.campaign_id; }

	// operations:
	virtual void         Load();
	virtual void         Prep();
	virtual void         Start();
	virtual void         ExecFrame();
	virtual void         Unload();

	virtual void         Clear();
	virtual void         CommitExpiredActions();
	virtual void         LockoutEvents(int seconds);
	virtual void         CheckPlayerGroup();
	void                 CreatePlanners();

	// accessors:
	const char* Name()         const { return name; }
	const char* Description()  const { return description; }
	const char* Path()         const { return path; }

	const char* Situation()    const { return situation; }
	const char* Orders()       const { return orders; }

	void                 SetSituation(const char* s) { situation = s; }
	void                 SetOrders(const char* o) { orders = o; }

	int                  GetPlayerTeamScore();
	List<MissionInfo>& GetMissionList() { return missions; }
	List<Combatant>& GetCombatants() { return combatants; }
	List<CombatZone>& GetZones() { return zones; }
	List<AStarSystem>& GetSystemList() { return systems; }
	List<CombatAction>& GetActions() { return actions; }
	List<CombatEvent>& GetEvents() { return events; }
	CombatEvent* GetLastEvent();

	CombatAction* FindAction(int id);

	int                  CountNewEvents() const;

	int                  GetPlayerIFF();
	CombatGroup* GetPlayerGroup() { return player_group; }
	void                 SetPlayerGroup(CombatGroup* pg);
	CombatUnit* GetPlayerUnit() { return player_unit; }
	void                 SetPlayerUnit(CombatUnit* pu);

	Combatant* GetCombatant(const char* name);
	CombatGroup* FindGroup(int iff, int type, int id);
	CombatGroup* FindGroup(int iff, int type, CombatGroup* near_group = 0);
	CombatGroup* FindStrikeTarget(int iff, CombatGroup* strike_group);

	AStarSystem* GetSystem(const char* sys);
	CombatZone* GetZone(const char* rgn);
	MissionInfo* CreateNewMission();
	void                 DeleteMission(int id);
	Mission* GetMission();
	Mission* GetMission(int id);
	Mission* GetMissionByFile(const char* filename);
	MissionInfo* GetMissionInfo(int id);
	MissionInfo* FindMissionTemplate(int msn_type, CombatGroup* player_group);
	void                 ReloadMission(int id);
	void                 LoadNetMission(int id, const char* net_mission);
	void                 StartMission();
	void                 RollbackMission();

	void                 SetCampaignId(int id);
	int                  GetCampaignId()   const { return campaign_id; }
	void                 SetMissionId(int id);
	int                  GetMissionId()    const { return mission_id; }
	//Bitmap* GetImage(int n) { return &image[n]; }
	double               GetTime()         const { return time; }
	double               GetStartTime()    const { return startTime; }
	void                 SetStartTime(double t) { startTime = t; }
	double               GetLoadTime()     const { return loadTime; }
	void                 SetLoadTime(double t) { loadTime = t; }
	double               GetUpdateTime()   const { return updateTime; }
	void                 SetUpdateTime(double t) { updateTime = t; }

	bool                 InCutscene()      const;
	bool                 IsDynamic()       const;
	bool                 IsTraining()      const;
	bool                 IsScripted()      const;
	bool                 IsSequential()    const;
	bool                 IsSaveGame()      const { return loaded_from_savegame; }
	void                 SetSaveGame(bool s) { loaded_from_savegame = s; }

	bool                 IsActive()        const { return status == CAMPAIGN_ACTIVE; }
	bool                 IsComplete()      const { return status == CAMPAIGN_SUCCESS; }
	bool                 IsFailed()        const { return status == CAMPAIGN_FAILED; }
	void                 SetStatus(int s);
	int                  GetStatus()       const { return status; }

	int                  GetAllCombatUnits(int iff, List<CombatUnit>& units);

	static void          Initialize();
	static void          Close();
	static ACampaign* GetCampaign();
	static List<ACampaign>&GetAllCampaigns();
	static int           GetLastCampaignId();
	static ACampaign* SelectCampaign(const char* name);
	static ACampaign* CreateCustomCampaign(const char* name, const char* path);

	static double        Stardate();

protected:
	void                 LoadCampaign(DataLoader* loader, bool full = false);
	void                 LoadTemplateList(DataLoader* loader);
	void                 LoadMissionList(DataLoader* loader);
	void                 LoadCustomMissions(DataLoader* loader);
	void                 ParseGroup(TermStruct* val, CombatGroup* force,CombatGroup* clone, const char* filename);
	void                 ParseAction(TermStruct* val, const char* filename);
	CombatGroup* CloneOver(CombatGroup* force, CombatGroup* clone, CombatGroup* group);
	void                 SelectDefaultPlayerGroup(CombatGroup* g, int type);
	TemplateList* GetTemplateList(int msn_type, int grp_type);

	// attributes:
	int                  campaign_id;
	int                  status;
	char                 filename[64];
	char                 path[64];
	Text                 name;
	Text                 description;
	Text                 situation;
	Text                 orders;
	//Bitmap               image[NUM_IMAGES];

	bool                 scripted;
	bool                 sequential;
	bool                 loaded_from_savegame;

	List<Combatant>      combatants;
	List<AStarSystem>     systems;
	List<CombatZone>     zones;
	List<CampaignPlan>   planners;
	List<MissionInfo>    missions;
	List<TemplateList>   templates;
	List<CombatAction>   actions;
	List<CombatEvent>    events;
	CombatGroup* player_group;
	CombatUnit* player_unit;

	int                  mission_id;
	Mission* mission;
	Mission* net_mission;

	double               time;
	double               loadTime;
	double               startTime;
	double               updateTime;
	int                  lockout;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
