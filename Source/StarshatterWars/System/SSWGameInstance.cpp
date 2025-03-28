// Fill out your copyright notice in the Description page of Project Settings.


#include "SSWGameInstance.h"
#include "GameFramework/Actor.h"
#include "../Space/Universe.h"
#include "../Space/Galaxy.h"
#include "../Foundation/DataLoader.h"
#include "../Game/Sim.h"
#include "../Game/GameDataLoader.h"
#include "../Game/AwardInfoLoader.h"
#include "../Game/CombatGroupLoader.h"

#include "../Screen/MenuDlg.h"
#include "../Screen/QuitDlg.h"
#include "../Screen/FirstRun.h"
#include "../Screen/CampaignScreen.h"

#include "../Game/PlayerSaveGame.h"
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
	bClearTables = false;
	
	InitializeDT(ObjectInitializer);

	InitializeMainMenuScreen(ObjectInitializer);
	InitializeCampaignScreen(ObjectInitializer);
	InitializeQuitDlg(ObjectInitializer);
	InitializeFirstRunDlg(ObjectInitializer);

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
	UWorld* World = GetWorld();

	FVector location = FVector::ZeroVector;
	FRotator rotate = FRotator::ZeroRotator;

	FActorSpawnParameters SpawnInfo;
	FName Name("Campaign Data");
	SpawnInfo.Name = Name;

	if (GameData == nullptr) {
		GameData = GetWorld()->SpawnActor<AGameDataLoader>(AGameDataLoader::StaticClass(), location, rotate, SpawnInfo);


		if (GameData)
		{
			UE_LOG(LogTemp, Log, TEXT("Game Data Loader Spawned"));
		}
		else {
			UE_LOG(LogTemp, Log, TEXT("Failed to Spawn Game Data Loader"));
		}
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Game Data Loader  already exists"));
	}

	//} else {
	//	UE_LOG(LogTemp, Log, TEXT("World not found"));
	//}		
}

void USSWGameInstance::GetCombatRosterData()
{
	UWorld* World = GetWorld();

	FVector location = FVector::ZeroVector;
	FRotator rotate = FRotator::ZeroRotator;

	FActorSpawnParameters SpawnInfo;
	FName Name("Combat Roster Data");
	SpawnInfo.Name = Name;

	if (CombatGroupData == nullptr) {
		CombatGroupData = GetWorld()->SpawnActor<ACombatGroupLoader>(ACombatGroupLoader::StaticClass(), location, rotate, SpawnInfo);


		if (CombatGroupData)
		{
			UE_LOG(LogTemp, Log, TEXT("Combat Group Info Loader Spawned"));
		}
		else {
			UE_LOG(LogTemp, Log, TEXT("Failed to Spawn Combat Group Data Loader"));
		}
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Combat Group Data Loader already exists"));
	}

	//} else {
	//	UE_LOG(LogTemp, Log, TEXT("World not found"));
	//}		
}

void USSWGameInstance::GetAwardInfoData()
{
	UWorld* World = GetWorld();

	FVector location = FVector::ZeroVector;
	FRotator rotate = FRotator::ZeroRotator;

	FActorSpawnParameters SpawnInfo;
	FName Name("Award Data");
	SpawnInfo.Name = Name;

	if (AwardData == nullptr) {
		AwardData = GetWorld()->SpawnActor<AAwardInfoLoader>(AAwardInfoLoader::StaticClass(), location, rotate, SpawnInfo);


		if (AwardData)
		{
			UE_LOG(LogTemp, Log, TEXT("Award Info Loader Spawned"));
		}
		else {
			UE_LOG(LogTemp, Log, TEXT("Failed to Spawn Award Info Data Loader"));
		}
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Award Info Data Loader already exists"));
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

	if(bClearTables) {
		CampaignDataTable->EmptyTable();
	}
	LoadGame("PlayerSaveSlot", 0);
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

	/*if (!bUniverseLoaded) {
		bUniverseLoaded = true;
		NewObject<UUniverse>();
		Sim = NewObject<USim>(); 
	}*/

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

void USSWGameInstance::InitializeDT(const FObjectInitializer& ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UDataTable> CampaignDataTableObject(TEXT("DataTable'/Game/Game/DT_Campaign.DT_Campaign'"));

	if (CampaignDataTableObject.Succeeded())
	{
		CampaignDataTable = CampaignDataTableObject.Object;
	}
}

void USSWGameInstance::InitializeMainMenuScreen(const FObjectInitializer& ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<UMenuDlg> MainMenuScreenWidget(TEXT("/Game/Screens/WB_MainMenu"));
	if (!ensure(MainMenuScreenWidget.Class != nullptr))
	{
		return;
	}
	MainMenuScreenWidgetClass = MainMenuScreenWidget.Class;
}

void USSWGameInstance::InitializeCampaignScreen(const FObjectInitializer& ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<UCampaignScreen> CampaignScreenWidget(TEXT("/Game/Screens/Campaign/WB_CampaignSelect"));
	if (!ensure(CampaignScreenWidget.Class != nullptr))
	{
		return;
	}
	CampaignScreenWidgetClass = CampaignScreenWidget.Class;
}

void USSWGameInstance::InitializeQuitDlg(const FObjectInitializer& ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<UQuitDlg> QuitDlgWidget(TEXT("/Game/Screens/WB_QuitDlg"));
	if (!ensure(QuitDlgWidget.Class != nullptr))
	{
		return;
	}
	QuitDlgWidgetClass = QuitDlgWidget.Class;
}

void USSWGameInstance::InitializeFirstRunDlg(const FObjectInitializer& ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<UFirstRun>FirstRunDlgWidget(TEXT("/Game/Screens/Main/WB_FirstRunDlg"));
	if (!ensure(FirstRunDlgWidget.Class != nullptr))
	{
		return;
	}
	FirstRunDlgWidgetClass = FirstRunDlgWidget.Class;
}

void USSWGameInstance::ShowMainMenuScreen()
{
	// Create widget
	MainMenuDlg = CreateWidget<UMenuDlg>(this, MainMenuScreenWidgetClass);
	// Add it to viewport
	MainMenuDlg->AddToViewport(100);

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(MainMenuDlg->TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}
	ShowQuitDlg();
	ShowFirstRunDlg();

	if (UGameplayStatics::DoesSaveGameExist("PlayerSaveSlot", 0)) {
		ToggleFirstRunDlg(false);
	}
	else
	{
		ToggleFirstRunDlg(true);
	}
}

void USSWGameInstance::ShowCampaignScreen()
{
	// Create widget
	if(!CampaignScreen) {
		// Create widget
		CampaignScreen = CreateWidget<UCampaignScreen>(this, CampaignScreenWidgetClass);
	}
	
	// Add it to viewport
	CampaignScreen->AddToViewport(101);

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(CampaignScreen->TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}
	ToggleCampaignScreen(true);
}

void USSWGameInstance::ShowQuitDlg()
{
	// Create widget
	QuitDlg = CreateWidget<UQuitDlg>(this, QuitDlgWidgetClass);
	// Add it to viewport
	QuitDlg->AddToViewport(101);

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(QuitDlg->TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}
	ToggleQuitDlg(false);
}

void USSWGameInstance::ShowFirstRunDlg()
{
	// Create widget
	FirstRunDlg = CreateWidget<UFirstRun>(this, FirstRunDlgWidgetClass);
	// Add it to viewport
	FirstRunDlg->AddToViewport(102);

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(FirstRunDlg->TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}
	ToggleFirstRunDlg(false);
}

void USSWGameInstance::ToggleQuitDlg(bool bVisible)
{
	if(QuitDlg) {
		if(bVisible) {
			QuitDlg->SetVisibility(ESlateVisibility::Visible);
		} else {
			QuitDlg->SetVisibility(ESlateVisibility::Collapsed);
		}	
	}
}

void USSWGameInstance::ToggleFirstRunDlg(bool bVisible)
{
	if (FirstRunDlg) {
		if (bVisible) {
			FirstRunDlg->SetVisibility(ESlateVisibility::Visible);
		}
		else {
			FirstRunDlg->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	if (MainMenuDlg) {
		MainMenuDlg->EnableMenuButtons(!bVisible);
	}
}

void USSWGameInstance::ToggleMenuButtons(bool bVisible)
{
	if (MainMenuDlg) {
		MainMenuDlg->EnableMenuButtons(bVisible);
	}
}

void USSWGameInstance::ToggleCampaignScreen(bool bVisible) {
	if (CampaignScreen) {
		if (bVisible) {
			CampaignScreen->SetVisibility(ESlateVisibility::Visible);
		}
		else {
			CampaignScreen->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void USSWGameInstance::SetGameMode(EMODE gm)
{

}

void USSWGameInstance::SaveGame(FString SlotName, int32 UserIndex, FS_PlayerGameInfo PlayerData)
{
	UPlayerSaveGame* SaveInstance = Cast<UPlayerSaveGame>(UGameplayStatics::CreateSaveGameObject(UPlayerSaveGame::StaticClass()));

	if (SaveInstance)
	{
		SaveInstance->PlayerInfo = PlayerData;

		UGameplayStatics::SaveGameToSlot(SaveInstance, SlotName, UserIndex);
	}
}

void USSWGameInstance::LoadGame(FString SlotName, int32 UserIndex)
{
	if (UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
	{
		UPlayerSaveGame* LoadedGame = Cast<UPlayerSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
		PlayerInfo = LoadedGame->PlayerInfo;
	}
}
