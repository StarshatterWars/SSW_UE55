// Fill out your copyright notice in the Description page of Project Settings.


#include "SSWGameInstance.h"
#include "GameFramework/Actor.h"
#include "../Space/Universe.h"
#include "../Space/Galaxy.h"
#include "../Foundation/DataLoader.h"
#include "Engine/World.h"

USSWGameInstance::USSWGameInstance(const FObjectInitializer& ObjectInitializer)
{
	bIsWindowed = false;
	bIsActive = false;
	bIsDeviceLost = false;
	bIsMinimized = false;
	bIsMaximized = false;
	bIgnoreSizeChange = false;
	bIsDeviceInitialized = false;
	bIsDeviceRestored = false;
	GameUniverse = nullptr;

	

	SetProjectPath();
	Init();
}

void USSWGameInstance::SetProjectPath()
{
	ProjectPath = FPaths::ProjectDir();
	ProjectPath.Append(TEXT("GameData/")); 

	UE_LOG(LogTemp, Log, TEXT("Setting Game Data Directory %s"), *ProjectPath);
}

FString USSWGameInstance::GetProjectPath()
{
	return ProjectPath;
}

void USSWGameInstance::Print(FString Msg, FString File)
{
	UE_LOG(LogTemp, Log, TEXT("%s :%s"), *Msg, *File);
}

void USSWGameInstance::SpawnUniverse()
{
	UWorld* World = GetWorld();

	FVector location = FVector::ZeroVector;
	FRotator rotate = FRotator::ZeroRotator;

	FActorSpawnParameters SpawnInfo;

	if (GameUniverse == nullptr) {
		GameUniverse = GetWorld()->SpawnActor<AUniverse>(AUniverse::StaticClass(), location, rotate, SpawnInfo);


		if (GameUniverse)
		{
			UE_LOG(LogTemp, Log, TEXT("Game Universe Spawned"));
		}
		else {
			UE_LOG(LogTemp, Log, TEXT("Failed to Spawn Game Universe"));
		}
	} 
	else {
		UE_LOG(LogTemp, Log, TEXT("Game Universe already exists"));
	}

	//} else {
	//	UE_LOG(LogTemp, Log, TEXT("World not found"));
	//}		
}

void USSWGameInstance::SpawnGalaxy()
{
	UWorld* World = GetWorld();

	FVector location = FVector::ZeroVector;
	FRotator rotate = FRotator::ZeroRotator;

	FActorSpawnParameters SpawnInfo;

	if (GameGalaxy == nullptr) {
		GameGalaxy = GetWorld()->SpawnActor<AGalaxy>(AGalaxy::StaticClass(), location, rotate, SpawnInfo);


		if (GameGalaxy)
		{
			UE_LOG(LogTemp, Log, TEXT("Game Galaxy Spawned"));
		}
		else {
			UE_LOG(LogTemp, Log, TEXT("Failed to Spawn Game Galaxy"));
		}
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Game Galaxy already exists"));
	}

	//} else {
	//	UE_LOG(LogTemp, Log, TEXT("World not found"));
	//}		
}

void USSWGameInstance::StartGame()
{
	//SpawnUniverse();
	//SpawnGalaxy();
}

void USSWGameInstance::Init()
{
	Status = EGAMESTATUS::OK;
	UE_LOG(LogTemp, Log, TEXT("Initializing Game\n."));

	if (Status == EGAMESTATUS::OK) {
		UE_LOG(LogTemp, Log, TEXT("\n  Initializing instance...\n"));
	}

	if (Status == EGAMESTATUS::OK) {
		UE_LOG(LogTemp, Log, TEXT("  Initializing content...\n"));
		InitContent();
	}
}

void USSWGameInstance::Shutdown()
{
	
}

bool USSWGameInstance::InitContent()
{
	DataLoader* loader = new DataLoader();
	
	loader->GetLoader();
	List<Text>  bundles;
	
	ProjectPath = FPaths::ProjectDir();
	ProjectPath.Append(TEXT("GameData/Content/"));

	loader->SetDataPath(ProjectPath);
	//loader->ListFiles("content*", bundles);

	ProjectPath = FPaths::ProjectDir();
	ProjectPath.Append(TEXT("GameData/"));
	loader->SetDataPath(ProjectPath);
	return true;
}

bool USSWGameInstance::InitGame()
{
	return false;
}
