/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Space
	FILE:         GameLoader.cpp
	AUTHOR:       Carlos Bott
*/

#include "GameLoader.h"
#include "../Space/Universe.h"
#include "../Game/Sim.h"
#include "../Game/Campaign.h"
#include "../Foundation/DataLoader.h"
#include "../Foundation/MusicController.h"

void AGameLoader::BeginPlay()
{
	InitializeGame();
	GetGameData();
	LoadMainMenu();
	//LoadGalaxy();
}

void AGameLoader::Tick(float DeltaTime)
{
}

AGameLoader::AGameLoader()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SetActorHiddenInGame(true);
	SetCanBeDamaged(false);
}

USSWGameInstance* AGameLoader::GetSSWGameInstance()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	return SSWInstance;
}

void AGameLoader::LoadGalaxy()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->SpawnGalaxy();
}

void AGameLoader::GetGameData()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->GetGameData();
}

void AGameLoader::InitializeGame()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	
	UUniverse* Universe;
	Universe = NewObject<UUniverse>();

	USim* Sim;
	Sim = NewObject<USim>();

	UCampaign* Campaign;
	Campaign = NewObject<UCampaign>();
	//UCampaign::Initialize();
}

void AGameLoader::LoadMainMenu()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->LoadMainMenuScreen();
}



