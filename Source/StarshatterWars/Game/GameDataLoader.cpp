/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         GameDataLoader.cpp
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Master Game Data Loader
*/



#include "GameDataLoader.h"
#include "../System/SSWGameInstance.h"


// Sets default values
AGameDataLoader::AGameDataLoader()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::AGameDataLoader()"));
}

// Called when the game starts or when spawned
void AGameDataLoader::BeginPlay()
{
	Super::BeginPlay();
	GetSSWInstance();
	InitializeCampaignData();
}

// Called every frame
void AGameDataLoader::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGameDataLoader::LoadGalaxyData()
{

}

void AGameDataLoader::GetSSWInstance()
{
	SSWInstance = (USSWGameInstance*)GetGameInstance();
}

void AGameDataLoader::InitializeCampaignData() {
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::LoadCampaignData"));

	FString ProjectPath = FPaths::ProjectDir();
	ProjectPath.Append(TEXT("GameData/Campaigns/"));
	FString FileName = ProjectPath;

	for (int i = 1; i < 6; i++) {		
		FileName = ProjectPath; 
		FileName.Append("0");
		FileName.Append(FString::FormatAsNumber(i));
		FileName.Append("/");
		FileName.Append("campaign.def");
		LoadCampaignData(TCHAR_TO_ANSI(*FileName));
	}
}

void AGameDataLoader::LoadCampaignData(const char* FileName)
{
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::LoadCampaignData"));
	SSWInstance->loader->GetLoader();
	SSWInstance->loader->SetDataPath(FileName);

	FString fs = FString(ANSI_TO_TCHAR(FileName));
	FString FileString;
	BYTE* block = 0;

	SSWInstance->loader->LoadBuffer(FileName, block, true);

	UE_LOG(LogTemp, Log, TEXT("Loading Campaign Data: %s"), *fs);

	if (FFileHelper::LoadFileToString(FileString, *fs, FFileHelper::EHashOptions::None))
	{
		UE_LOG(LogTemp, Log, TEXT("%s"), *FileString);

		const char* result = TCHAR_TO_ANSI(*FileString);
	}

	Parser parser(new BlockReader((const char*)block));

	Term* term = parser.ParseTerm();
}

