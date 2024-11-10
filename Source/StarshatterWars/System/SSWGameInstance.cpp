// Fill out your copyright notice in the Description page of Project Settings.


#include "SSWGameInstance.h"
#include "GameFramework/Actor.h"
#include "../Space/Universe.h"
#include "../Space/Galaxy.h"
#include "../Foundation/DataLoader.h"
#include "../Game/Sim.h"
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
	Sim = nullptr;

	if (GameUniverse == nullptr)
		NewObject<UUniverse>();
	if (Sim == nullptr)
		Sim = NewObject<USim>();

	static ConstructorHelpers::FObjectFinder<UDataTable> StarsDataTableObject(TEXT("DataTable'/Game/Game/DT_Stars.DT_Stars'"));

	if (StarsDataTableObject.Succeeded())
	{
		StarsDataTable = StarsDataTableObject.Object;
		//StarsDataTable->EmptyTable();
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Failed to get Stars Data Table"));
	}

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

void USSWGameInstance::SpawnGalaxy()
{
	UWorld* World = GetWorld();

	FVector location = FVector::ZeroVector;
	FRotator rotate = FRotator::ZeroRotator;

	FActorSpawnParameters SpawnInfo;
	FName Name("Starshatter Galaxy");
	SpawnInfo.Name = Name;

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

void USSWGameInstance::GetCampaignData()
{

}

void USSWGameInstance::StartGame()
{
	//SpawnUniverse();
	//SpawnGalaxy();
}

void USSWGameInstance::Init()
{
	if (!DataLoader::GetLoader())
		DataLoader::Initialize();

	loader = DataLoader::GetLoader();
	
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

FString
USSWGameInstance::GetEmpireNameFromType(EEMPIRE_NAME emp)
{
	FString empire_name;

	switch (emp)
	{
	case EEMPIRE_NAME::Terellian_Alliance:
		empire_name = "Terellian Alliance";
		break;
	case EEMPIRE_NAME::Marakan_Hegemony:
		empire_name = "Marakan Hegemony";
		break;
	case EEMPIRE_NAME::Dantari_Separatists:
		empire_name = "Dantari Separatists";
		break;
	case EEMPIRE_NAME::Other:
		empire_name = "Other";
		break;
	case EEMPIRE_NAME::INDEPENDENT_SYSTEMS:
		empire_name = "Independent Systems";
		break;
	default:
		empire_name = "Other";
		break;
	}
	return empire_name;
}