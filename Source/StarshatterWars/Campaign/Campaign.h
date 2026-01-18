/*  Project Starshatter 4.5
	Fractal Dev Studios
	Copyright © 2025. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         Campaign.h
	AUTHOR:       Carlos Bott

	UNREAL PORT:
	- Maintains all variables and methods (names, signatures, members).
	- Uses UE-compatible shims for Text, List, Bitmap, TermStruct.
*/

#pragma once

// Original includes mapped to Unreal-compatible shims:
#include "Types.h"
//#include "Bitmap.h"
#include "Geometry.h"
#include "Text.h"
#include "Term.h"
#include "List.h"

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
class MissionTemplate;
class StarSystem;

// +--------------------------------------------------------------------+

class MissionInfo
{
public:
	static const char* TYPENAME() { return "MissionInfo"; }

	MissionInfo();
	~MissionInfo();

	int operator == (const MissionInfo& m) const { return id == m.id; }
	int operator <  (const MissionInfo& m) const { return id < m.id; }
	int operator <= (const MissionInfo& m) const { return id <= m.id; }

	bool     IsAvailable();

	int      id = 0;
	FString     name;
	FString     player_info;
	FString     description;
	FString     system;
	FString     region;
	FString     script;
	int      start = 0;
	int      type = 0;

	int      min_rank = 0;
	int      max_rank = 100;
	int      action_id = 0;
	int      action_status = 0;
	int      exec_once = 0;
	int      start_before = 0;
	int      start_after = 0;

	Mission* mission = nullptr;
};

class TemplateList
{
public:
	static const char* TYPENAME() { return "TemplateList"; }

	TemplateList();
	~TemplateList();

	int               mission_type = 0;
	int               group_type = 0;
	int               index = 0;
	List<MissionInfo> missions;
};

// +--------------------------------------------------------------------+

class Campaign
{
public:
	static const char* TYPENAME() { return "Campaign"; }

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

	Campaign(int id, const char* name = 0);
	Campaign(int id, const char* name, const char* path);
	virtual ~Campaign();

	int operator == (const Campaign& s) const { return name == s.name; }
	int operator <  (const Campaign& s) const { return campaign_id < s.campaign_id; }

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
	FString Name() const { return name; }
	FString Description()  const { return description; }
	FString Path()         const { return path; }

	FString Situation()    const { return situation; }
	FString Orders()       const { return orders; }

	void                 SetSituation(const char* s) { situation = s; }
	void                 SetOrders(const char* o) { orders = o; }

	int                  GetPlayerTeamScore();
	List<MissionInfo>& GetMissionList() { return missions; }
	List<Combatant>& GetCombatants() { return combatants; }
	List<CombatZone>& GetZones() { return zones; }
	List<StarSystem>& GetSystemList() { return systems; }
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

	StarSystem* GetSystem(const char* sys);
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
	static Campaign* GetCampaign();
	static List<Campaign>& GetAllCampaigns();
	static int           GetLastCampaignId();
	static Campaign* SelectCampaign(const char* name);
	static Campaign* CreateCustomCampaign(const char* name, const char* path);

	static double        Stardate();

protected:
	void                 LoadCampaign(DataLoader* loader, bool full = false);
	void                 LoadTemplateList(DataLoader* loader);
	void                 LoadMissionList(DataLoader* loader);
	void                 LoadCustomMissions(DataLoader* loader);
	void ParseGroup(TermStruct* val,
					CombatGroup* force,
					CombatGroup* clone,
					const char* filename);
	void ParseAction(TermStruct* val,
					 const char* filename);
					 CombatGroup* CloneOver(CombatGroup* force,
					 CombatGroup* clone,
					 CombatGroup* group);
	void                 SelectDefaultPlayerGroup(CombatGroup* g, int type);
	TemplateList* GetTemplateList(int msn_type, int grp_type);

	// attributes:
	int                  campaign_id = 0;
	int                  status = CAMPAIGN_INIT;
	FString              filename;
	FString              path;
	FString              name;
	FString              description;
	FString              situation;
	FString              orders;
	//Bitmap               image[NUM_IMAGES];

	bool                 scripted = false;
	bool                 sequential = false;
	bool                 loaded_from_savegame = false;

	List<Combatant>      combatants;
	List<StarSystem>     systems;
	List<CombatZone>     zones;
	List<CampaignPlan>   planners;
	List<MissionInfo>    missions;
	List<TemplateList>   templates;
	List<CombatAction>   actions;
	List<CombatEvent>    events;
	CombatGroup* player_group = nullptr;
	CombatUnit* player_unit = nullptr;

	int                  mission_id = 0;
	Mission* mission = nullptr;
	Mission* net_mission = nullptr;

	double               time = 0.0;
	double               loadTime = 0.0;
	double               startTime = 0.0;
	double               updateTime = 0.0;
	int                  lockout = 0;
};
