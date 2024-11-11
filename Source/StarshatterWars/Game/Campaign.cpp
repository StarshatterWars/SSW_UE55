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

void
UCampaign::Initialize()
{	
	UE_LOG(LogTemp, Log, TEXT("UCampaign::Initialize()"));

	FString ProjectPath = FPaths::ProjectDir();
	ProjectPath.Append(TEXT("GameData/Campaigns/"));
	FString FileName = ProjectPath;

	DataLoader* loader = DataLoader::GetLoader();
	
	//loader->SetDataPath(FileName);
	//const char* result = TCHAR_TO_ANSI(*FileName);

	
	for (int i = 1; i < 100; i++) {
				
		const char* result = TCHAR_TO_ANSI(*FileName);
		
		if(i >= 10) {
			FileName = ProjectPath; 
			FileName.Append(FString::FormatAsNumber(i));
		}
		else {
			
			FileName = ProjectPath; FileName.Append("0");
			FileName.Append(FString::FormatAsNumber(i));
		}
		
		UE_LOG(LogTemp, Log, TEXT("Loading Campaign: %s"), *FileName);
		loader->UseFileSystem(true);
		loader->SetDataPath(FileName);
		
		if (loader->FindFile("campaign.def")) {
			char txt[256];
			sprintf_s(txt, "Dynamic Campaign %02d", i);
			
			UCampaign* c;
			c = NewObject<UCampaign>();
			c->CampaignSet(i, txt);

			if (c) {
				campaigns.insertSort(c);
			}
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

}

void UCampaign::Clear()
{

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
