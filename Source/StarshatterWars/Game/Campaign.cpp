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

static List<ACampaign>   campaigns;
static ACampaign* current_campaign = 0;

// Sets default values
ACampaign::ACampaign()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ACampaign::Load()
{

}

void ACampaign::Prep()
{

}

void ACampaign::Start()
{

}

void ACampaign::ExecFrame()
{

}

void ACampaign::Unload()
{

}

void ACampaign::Clear()
{

}

void ACampaign::CommitExpiredActions()
{

}

void ACampaign::LockoutEvents(int seconds)
{

}

void ACampaign::CheckPlayerGroup()
{

}

void ACampaign::CreatePlanners()
{

}

int ACampaign::GetPlayerTeamScore()
{
	return 0;
}

// Called when the game starts or when spawned
void ACampaign::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACampaign::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACampaign::Initialize()
{
}

void ACampaign::Close()
{
}

ACampaign* ACampaign::GetCampaign()
{
	return current_campaign;
}

List<ACampaign>& ACampaign::GetAllCampaigns()
{
	return campaigns;
}

int ACampaign::GetLastCampaignId()
{
	int result = 0;

	for (int i = 0; i < campaigns.size(); i++) {
		ACampaign* c = campaigns.at(i);

		if (c->IsDynamic() && c->GetCampaignId() > result) {
			result = c->GetCampaignId();
		}
	}

	return result;
}

ACampaign* ACampaign::SelectCampaign(const char* name)
{
	ACampaign* c = 0;
	ListIter<ACampaign>   iter = campaigns;

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

ACampaign* ACampaign::CreateCustomCampaign(const char* name, const char* path)
{
	return nullptr;
}

AStarSystem* ACampaign::GetSystem(const char* sys)
{
	return nullptr;
}

bool ACampaign::IsDynamic() const
{
	return campaign_id >= (int) CAMPAIGN_CONSTANTS::DYNAMIC_CAMPAIGN &&
		campaign_id < (int) CAMPAIGN_CONSTANTS::SINGLE_MISSIONS;
}
