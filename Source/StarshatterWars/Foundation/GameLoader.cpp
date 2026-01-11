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
#include "Kismet/GameplayStatics.h"
#include "Misc/Guid.h"
#include "HAL/PlatformTime.h"
#include "../System/UniverseSaveGame.h"
#include "../Foundation/MusicController.h"

void AGameLoader::BeginPlay()
{
	
	InitializeGame();
	
	LoadOrCreateUniverse();
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

void AGameLoader::LoadOrCreateUniverse()
{
	const FString Slot = GetUniverseSlotName();
	constexpr int32 UserIndex = 0;

	// Load if exists
	if (UGameplayStatics::DoesSaveGameExist(Slot, UserIndex))
	{
		USaveGame* Raw = UGameplayStatics::LoadGameFromSlot(Slot, UserIndex);
		UniverseSave = Cast<UUniverseSaveGame>(Raw);
	}

	// Create if missing/corrupt
	if (!UniverseSave)
	{
		UniverseSave = Cast<UUniverseSaveGame>(
			UGameplayStatics::CreateSaveGameObject(UUniverseSaveGame::StaticClass())
		);

		UniverseSave->UniverseId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphensLower);

		// Generate seed ONCE
		UniverseSave->UniverseSeed = FPlatformTime::Cycles64();
		UniverseSave->UniverseTimeSeconds = 0.0;

		UGameplayStatics::SaveGameToSlot(UniverseSave, Slot, UserIndex);
	}

	UE_LOG(LogTemp, Log, TEXT("Universe loaded: Id=%s Seed=%llu Time=%.2f"),
		*UniverseSave->UniverseId,
		(unsigned long long)UniverseSave->UniverseSeed,
		UniverseSave->UniverseTimeSeconds);

	// Push into GameInstance (runtime source of truth)
	if (USSWGameInstance* GI = GetSSWGameInstance())
	{
		// You will add these UPROPERTY fields to the GameInstance:
		GI->UniverseId = UniverseSave->UniverseId;
		GI->UniverseSeed = UniverseSave->UniverseSeed;
		GI->UniverseTimeSeconds = UniverseSave->UniverseTimeSeconds;
	}
}




