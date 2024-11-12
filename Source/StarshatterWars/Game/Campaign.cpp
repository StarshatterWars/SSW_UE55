/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         Campaign.cpp
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Campaign defines a strategic military scenario.
*/



#include "Campaign.h"
#include "CampaignPlanStrategic.h"
#include "CampaignPlanAssignment.h"
#include "CampaignPlanEvent.h"
#include "CampaignPlanMission.h"
#include "CampaignPlanMovement.h"
#include "CampaignSituationReport.h"
#include "CampaignSaveGame.h"
#include "Combatant.h"
#include "CombatAction.h"
#include "CombatEvent.h"
#include "CombatGroup.h"
#include "CombatRoster.h"
#include "CombatUnit.h"
#include "CombatZone.h"
#include "../Space/Galaxy.h"
#include "Mission.h"
#include "../Space/StarSystem.h"
#include "../StarshatterWars.h"
#include "PlayerData.h"

#include "../System/Game.h"
//#include "Bitmap.h"
#include "../Foundation/DataLoader.h"
#include "../Foundation/ParseUtil.h"
#include "../Foundation/Random.h"
#include "../Foundation/FormatUtil.h"
#include "../Foundation/GameLoader.h"
#include "../Game/GameDataLoader.h"
#include "../System/SSWGameInstance.h"

static List<UCampaign>   campaigns;
static UCampaign* current_campaign = 0;

UCampaign::UCampaign()
{
	
}

UCampaign::UCampaign(int id, const char* n /*= 0*/)
{
	campaign_id = id;
	name = n;
	mission_id = -1;
	mission = 0;
	net_mission = 0;

	scripted = false;
	sequential = false;
	time = 0;
	startTime = 0;
	loadTime = 0;

	player_group = 0;
	player_unit = 0;
	status = CAMPAIGN_INIT;
	lockout = 0;
	loaded_from_savegame = false;

	Load();
}

UCampaign::UCampaign(int id, const char* n, const char* p)
{
	campaign_id = id;
	name = n;
	mission_id = -1;
	mission = 0;
	net_mission = 0;

	scripted = false;
	sequential = false;
	time = 0;
	startTime = 0;
	loadTime = 0;

	player_group = 0;
	player_unit = 0;
	status = CAMPAIGN_INIT;
	lockout = 0;
	loaded_from_savegame = false;

	path = p;
	Load();
}

void UCampaign::CampaignSet(int id, const char* n)
{
	campaign_id = id;
	name = n;
	mission_id = -1;
	mission = 0;
	net_mission = 0;

	scripted = false;
	sequential = false;
	time = 0;
	startTime = 0;
	loadTime = 0;

	player_group = 0;
	player_unit = 0;
	status = CAMPAIGN_INIT;
	lockout = 0;
	loaded_from_savegame = false;
	UE_LOG(LogTemp, Log, TEXT("Initializing %s: %02d"), *FString(n), id);

	Load();
}

void UCampaign::CampaignSet(int id, const char* n, const char* p)
{
	campaign_id = id;
	name = n;
	mission_id = -1;
	mission = 0;
	net_mission = 0;

	scripted = false;
	sequential = false;
	time = 0;
	startTime = 0;
	loadTime = 0;

	player_group = 0;
	player_unit = 0;
	status = CAMPAIGN_INIT;
	lockout = 0;
	loaded_from_savegame = false;

	path = p;
	Load();
}

// +--------------------------------------------------------------------+

UCampaign::~UCampaign()
{
	//for (int i = 0; i < NUM_IMAGES; i++)
	//	image[i].ClearImage();

	//delete net_mission;

	//actions.destroy();
	//events.destroy();
	//missions.destroy();
	//templates.destroy();
	//planners.destroy();
	//zones.destroy();
	//combatants.destroy();
}

// +--------------------------------------------------------------------+



void
UCampaign::Initialize()
{		
	DataLoader* loader = DataLoader::GetLoader();
	
	UE_LOG(LogTemp, Log, TEXT("UCampaign::Initialize()"));
	
	FString ProjectPath = FPaths::ProjectDir();
	ProjectPath.Append(TEXT("GameData/Campaigns/"));
	FString FileName = ProjectPath;
	
	for (int i = 1; i < 6; i++) {
				
		const char* result = TCHAR_TO_ANSI(*FileName);
		FileName = ProjectPath; FileName.Append("0");
		FileName.Append(FString::FormatAsNumber(i));
		FileName.Append("/");
		
		UCampaign* c;
		c = NewObject<UCampaign>();
		c->CampaignSet(i, "Dynamic Campaign");

		if (c) {
			campaigns.insertSort(c);
		}
	}

	UCampaign* c;
	c = NewObject<UCampaign>();
	c->CampaignSet(SINGLE_MISSIONS, "Single Missions");
	if (c) {
		campaigns.insertSort(c);
		current_campaign = c;
	}

	c->CampaignSet(MULTIPLAYER_MISSIONS, "Multiplayer Missions");
	if (c) {
		campaigns.insertSort(c);
	}

	c->CampaignSet(CUSTOM_MISSIONS, "Custom Missions");
	if (c) {
		campaigns.insertSort(c);
	}
}

void UCampaign::Tick(float DeltaTime)
{

}

bool UCampaign::IsTickable() const
{
	return false;
}

bool UCampaign::IsTickableInEditor() const
{
	return false;
}

bool UCampaign::IsTickableWhenPaused() const
{
	return false;
}

TStatId UCampaign::GetStatId() const
{
	return TStatId();
}

UWorld* UCampaign::GetWorld() const
{
	return GetOuter()->GetWorld();
}

void UCampaign::Load()
{
	// first, unload any existing data:
	Unload();
	FString ProjectPath = FPaths::ProjectDir();
	ProjectPath.Append(TEXT("GameData/Campaigns/"));
	FString FileName = ProjectPath;

	if (!path[0]) {
		// then load the campaign from files:
		switch (campaign_id) {
		case SINGLE_MISSIONS:      
			ProjectPath.Append(TEXT("GameData/Missions/"));
			break;

		case CUSTOM_MISSIONS:      
			ProjectPath.Append(TEXT("GameData/Mods/Missions/"));
			break;

		case MULTIPLAYER_MISSIONS: 
			ProjectPath.Append(TEXT("GameData/Multiplayer/"));
			break;

		default:                   
			ProjectPath.Append(TEXT("GameData/Campaigns/"));
			FileName = ProjectPath; FileName.Append("0");
			FileName.Append(FString::FormatAsNumber(campaign_id));
			FileName.Append("/");
			FileName.Append("campaign.def");
			break;
		}
	}
	
	//loader->UseFileSystem(true);
	//loader->SetDataPath(FileName);
	
	systems.clear();

	/*if (loader->FindFile("zones.def"))
		zones.append(CombatZone::Load("zones.def"));

	for (int i = 0; i < zones.size(); i++) {
		Text s = zones[i]->System();
		bool found = false;

		for (int n = 0; !found && n < systems.size(); n++) {
			if (s == systems[n]->Name())
				found = true;
		}

		if (!found)
			systems.append(AGalaxy::GetInstance()->GetSystem(s));
	}*/

	
	
	gl->LoadCampaignData(TCHAR_TO_ANSI(*FileName));

	/*if (campaign_id == CUSTOM_MISSIONS) {
		loader->SetDataPath(path);
		LoadCustomMissions(loader);
	}
	else {
		bool found = false;

		if (loader->FindFile("missions.def")) {
			loader->SetDataPath(path);
			LoadMissionList(loader);
			found = true;
		}

		if (loader->FindFile("templates.def")) {
			loader->SetDataPath(path);
			LoadTemplateList(loader);
			found = true;
		}

		if (!found) {
			loader->SetDataPath(path);
			LoadCustomMissions(loader);
		}
	}

	loader->UseFileSystem(true);
	loader->SetDataPath(path);

	if (loader->FindFile("image.pcx")) {
		loader->LoadBitmap("image.pcx", image[0]);
		loader->LoadBitmap("selected.pcx", image[1]);
		loader->LoadBitmap("unavail.pcx", image[2]);
		loader->LoadBitmap("banner.pcx", image[3]);
	}

	loader->SetDataPath(0);
	loader->UseFileSystem(true);*/
}

void UCampaign::Prep()
{

}

void UCampaign::Start()
{

}

void UCampaign::ExecFrame()
{

}

void UCampaign::Unload()
{
	SetStatus(CAMPAIGN_INIT);

	Game::ResetGameTime();
	AStarSystem::SetBaseTime(0);

	startTime = Stardate();
	loadTime = startTime;
	lockout = 0;

	//for (int i = 0; i < NUM_IMAGES; i++)
	//	image[i].ClearImage();

	Clear();

	//zones.destroy();
}

void UCampaign::Clear()
{
	//missions.destroy();
	//planners.destroy();
	//combatants.destroy();
	//events.destroy();
	//actions.destroy();

	player_group = 0;
	player_unit = 0;

	updateTime = time;
}

void UCampaign::CommitExpiredActions()
{

}

void UCampaign::LockoutEvents(int seconds)
{

}

void UCampaign::CheckPlayerGroup()
{

}

void UCampaign::CreatePlanners()
{

}

int UCampaign::GetPlayerTeamScore()
{
	return 0;
}

void UCampaign::SetStatus(int s)
{
}

void UCampaign::Close()
{
	Print("Campaign::Close() - destroying all campaigns\n");
	current_campaign = 0;
	//campaigns.destroy();
}

UCampaign* UCampaign::GetCampaign()
{
	return current_campaign;
}

List<UCampaign>& UCampaign::GetAllCampaigns()
{
	return campaigns;
}

int UCampaign::GetLastCampaignId()
{
	int result = 0;

	for (int i = 0; i < campaigns.size(); i++) {
		UCampaign* c = campaigns.at(i);

		if (c->IsDynamic() && c->GetCampaignId() > result) {
			result = c->GetCampaignId();
		}
	}

	return result;
}

UCampaign* UCampaign::SelectCampaign(const char* name)
{
	UCampaign* c = 0;
	ListIter<UCampaign>   iter = campaigns;

	while (++iter && !c) {
		if (!_stricmp(iter->Name(), name))
			c = iter.value();
	}

	if (c) {
		Print("Campaign: Selected '%s'\n", c->Name());
		current_campaign = c;
	}
	else {
		Print("Campaign: could not find '%s'\n", name);
	}

	return c;
}

UCampaign* UCampaign::CreateCustomCampaign(const char* name, const char* path)
{
	return nullptr;
}

double UCampaign::Stardate()
{
	return AStarSystem::GetStardate();
}

void UCampaign::LoadCampaign(FString n, bool full /*= false*/)
{		
	DataLoader* loader = DataLoader::GetLoader();

	BYTE* block = 0;
	FString fn = "campaign.def";
	n.Append(fn);
	FString FileString;

	const char* result = TCHAR_TO_ANSI(*n);

	UE_LOG(LogTemp, Log, TEXT("Loading Campaign %s"), *FString(n));
	if (FFileHelper::LoadFileToString(FileString, *n, FFileHelper::EHashOptions::None))
	{
		UE_LOG(LogTemp, Log, TEXT("%s"), *FileString);
	}

	loader->SetDataPath(n);
	loader->LoadBuffer(result, block, true);
	//loader->UseFileSystem(true);
	Parser parser(new BlockReader((const char*)block));

	Term* term = parser.ParseTerm();
}

CombatGroup* UCampaign::FindGroup(int iff, int type, int id)
{
	return nullptr;
}

CombatGroup* UCampaign::FindGroup(int iff, int type, CombatGroup* near_group)
{
	return nullptr;
}

CombatGroup* UCampaign::FindStrikeTarget(int iff, CombatGroup* strike_group)
{
	return nullptr;
}

AStarSystem* UCampaign::GetSystem(const char* sys)
{
	return nullptr;
}

CombatZone* UCampaign::GetZone(const char* rgn)
{
	return nullptr;
}

bool UCampaign::IsDynamic() const
{
	return campaign_id >= (int) CAMPAIGN_CONSTANTS::DYNAMIC_CAMPAIGN &&
		campaign_id < (int) CAMPAIGN_CONSTANTS::SINGLE_MISSIONS;
}

int UCampaign::GetPlayerIFF()
{
	return 0;
}
