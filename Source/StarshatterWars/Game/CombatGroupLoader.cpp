/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         CombatGroupLoader.cpp
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Loader and Parser class for initial generation of Combat Group Data Table
	Will not be used after Dable Table is Generated.
*/


#include "CombatGroupLoader.h"


// Sets default values
ACombatGroupLoader::ACombatGroupLoader()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACombatGroupLoader::BeginPlay()
{
	Super::BeginPlay();
	GetSSWInstance();
	LoadCombatRoster();
}

// Called every frame
void ACombatGroupLoader::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

//TArray<FString> Filelist = {
//	"Alliance.def", "Dantari.def", "Haiche.def", "Hegemony.def", "Pirates.def"
//}
// +-------------------------------------------------------------------+

void ACombatGroupLoader::LoadCombatRoster()
{
	UE_LOG(LogTemp, Log, TEXT("ACombatGroupLoader::LoadCombatRoster()"));
	FString ProjectPath = FPaths::ProjectDir();
	ProjectPath.Append(TEXT("GameData/Campaigns/"));
	FString PathName = ProjectPath;

	TArray<FString> output;
	output.Empty();

	List<Text> files;

	FString path = PathName + "*.def";
	FFileManagerGeneric::Get().FindFiles(output, *path, true, false);

	for (int i = 0; i < output.Num(); i++) {
		char* filename = TCHAR_TO_ANSI(*output[i]);

		LoadOrderOfBattle(filename);
	}	
}

void ACombatGroupLoader::LoadOrderOfBattle(const char* filename)
{
	UE_LOG(LogTemp, Log, TEXT("Loading Order of Battle Data: %s"), *FString(filename));
}

void ACombatGroupLoader::GetSSWInstance()
{
	SSWInstance = (USSWGameInstance*)GetGameInstance();
}

