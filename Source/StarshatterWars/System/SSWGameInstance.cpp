// Fill out your copyright notice in the Description page of Project Settings.


#include "SSWGameInstance.h"
#include "GameFramework/Actor.h"
#include "../Space/Universe.h"
#include "../Space/Galaxy.h"
#include "../Foundation/DataLoader.h"
#include "../Game/Sim.h"
#include "../Game/GameDataLoader.h"
#include "../Game/AwardInfoLoader.h"

#include "../Screen/MenuDlg.h"
#include "../Screen/QuitDlg.h"
#include "../Screen/FirstRun.h"
#include "../Screen/CampaignScreen.h"
#include "../Screen/OperationsScreen.h"
#include "../Screen/MissionLoading.h"
#include "../Screen/CampaignLoading.h"

#include "../Game/PlayerSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UObjectGlobals.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/Package.h" // For data asset support

#include "../Foundation/MusicController.h"
#include "AudioDevice.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

#undef UpdateResource
#undef PlaySound

template <typename TEnum>
FString EnumToDisplayString(TEnum EnumValue)
{
	static_assert(TIsEnum<TEnum>::Value, "EnumToDisplayNameString only works with UENUMS.");

	UEnum* EnumPtr = StaticEnum<TEnum>();
	if (!EnumPtr) return TEXT("Invalid");

	return EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(EnumValue)).ToString();
}

USSWGameInstance::USSWGameInstance(const FObjectInitializer& ObjectInitializer) 
{
	InitializeDT(ObjectInitializer);
	InitializeMainMenuScreen(ObjectInitializer);
	InitializeCampaignScreen(ObjectInitializer);
	InitializeCampaignLoadingScreen(ObjectInitializer);
	InitializeOperationsScreen(ObjectInitializer);
	InitializeMissionBriefingScreen(ObjectInitializer);
	InitializeQuitDlg(ObjectInitializer);
	InitializeFirstRunDlg(ObjectInitializer);
}

void USSWGameInstance::SetProjectPath()
{
	ProjectPath = FPaths::ProjectDir();
	ProjectPath.Append(TEXT("GameData/")); 

	UE_LOG(LogTemp, Log, TEXT("Setting Game Data Directory %s"), *ProjectPath);
}

void USSWGameInstance::StartGameTimers()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(TimerHandle, this, &USSWGameInstance::OnGameTimerTick, 1.0f, true);
	}
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

void USSWGameInstance::GetGameData()
{
	UWorld* World = GetWorld();

	FVector location = FVector::ZeroVector;
	FRotator rotate = FRotator::ZeroRotator;

	FActorSpawnParameters SpawnInfo;
	FName Name("Game Data");
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

void USSWGameInstance::StartGame()
{
	//SpawnUniverse();
	//SpawnGalaxy();
}

void USSWGameInstance::LoadMainMenuScreen()
{
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
			PlayerController->bShowMouseCursor = false; UGameplayStatics::OpenLevel(this, "MainMenu");
		}
	}
}

void USSWGameInstance::LoadTransitionScreen()
{
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
			PlayerController->bShowMouseCursor = false; UGameplayStatics::OpenLevel(this, "Transition");
		}
	}
}

void USSWGameInstance::LoadOperationsScreen()
{
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
			PlayerController->bShowMouseCursor = false; UGameplayStatics::OpenLevel(this, "Operations");
		}
	}
}

void USSWGameInstance::LoadMissionBriefingScreen()
{
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
			PlayerController->bShowMouseCursor = false; UGameplayStatics::OpenLevel(this, "MissionBriefing");
		}
	}
}

void USSWGameInstance::LoadCampaignScreen()
{
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
			PlayerController->bShowMouseCursor = false; UGameplayStatics::OpenLevel(this, "Campaign");
		}
	}
}

void USSWGameInstance::LoadGameLevel(FString LevelName)
{
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameAndUI InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
			PlayerController->bShowMouseCursor = false; UGameplayStatics::OpenLevel(this, FName(LevelName));
		}
	}
}

void USSWGameInstance::Init()
{
	Super::Init();
	bIsWindowed = false;
	bIsGameActive = false;
	bIsDeviceLost = false;
	bIsMinimized = false;
	bIsMaximized = false;
	bIgnoreSizeChange = false;
	bIsDeviceInitialized = false;
	bIsDeviceRestored = false;

	PlayerSaveName = "PlayerSaveSlot";
	PlayerSaveSlot = 0;

	FDateTime GameDate(2228, 1, 1);
	SetGameTime(GameDate.ToUnixTimestamp());
	SetProjectPath();

	CampaignData.SetNum(5); // number of campaigns

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

	if (UGameplayStatics::DoesSaveGameExist(PlayerSaveName, PlayerSaveSlot)) {
		LoadGame(PlayerSaveName, PlayerSaveSlot);
		UE_LOG(LogTemp, Log, TEXT("Player Name: %s"), *PlayerInfo.Name);

		if (PlayerInfo.Campaign >= 0) {
			ReadCampaignData();
		}
	}
	ReadCombatRosterData();
	//CreateOOBTable();
	//ExportDataTableToCSV(OrderOfBattleDataTable, TEXT("OOBExport.csv"));
}

void USSWGameInstance::ReadCampaignData()
{
	UE_LOG(LogTemp, Log, TEXT("USSWGameInstance::ReadCampaignData()"));
	static const FString ContextString(TEXT("ReadDataTable"));
	TArray<FS_Campaign*> AllRows;
	CampaignDataTable->GetAllRows<FS_Campaign>(ContextString, AllRows);

	int index = 0;
	for (FS_Campaign* Row : AllRows)
	{
		if (Row)
		{
			UE_LOG(LogTemp, Log, TEXT("Campaign Name: %s"), *Row->Name);
			UE_LOG(LogTemp, Log, TEXT("Campaign Available: %s"), (Row->bAvailable ? TEXT("true") : TEXT("false")));
			CampaignData[index] = *Row;
			CampaignData[index].Orders.SetNum(4);
			index++;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to load Campaign!"));
		}
	}
	SetActiveCampaign(CampaignData[PlayerInfo.Campaign]);
	FString NewCampaign = GetActiveCampaign().Name;
	UE_LOG(LogTemp, Log, TEXT("Active Campaign: %s"), *NewCampaign);
}

void USSWGameInstance::ReadCombatRosterData() {
	UE_LOG(LogTemp, Log, TEXT("USSWGameInstance::ReadCombatRosterData()"));
	static const FString ContextString(TEXT("ReadDataTable"));
	TArray<FS_CombatGroup*> AllRows;
	CombatGroupDataTable->GetAllRows<FS_CombatGroup>(ContextString, AllRows);
	CombatRosterData.Empty();
	for (FS_CombatGroup* Row : AllRows)
	{
		if (Row)
		{
			UE_LOG(LogTemp, Log, TEXT("Group Name: %s"), *Row->Name);
			
			CombatRosterData.Add(*Row);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to load Combat Roster!"));
		}
	}
}

void USSWGameInstance::SetActiveOOBForce(FS_OOBForce& Force) 
{
	CurrentForce = Force;
}

FS_OOBForce USSWGameInstance::GetActiveOOBForce() {
	return CurrentForce;
}

void USSWGameInstance::Shutdown()
{
	Super::Shutdown();

	if (FAudioDevice* AudioDevice = GEngine->GetMainAudioDeviceRaw())
	{
		AudioDevice->Flush(nullptr); // Stops all active sounds immediately
	}
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
	case EEMPIRE_NAME::Terellian:
		empire_name = "Terellian Alliance";
		break;
	case EEMPIRE_NAME::Marakan:
		empire_name = "Marakan Hegemony";
		break;
	case EEMPIRE_NAME::Independent:
		empire_name = "Independent System";
		break;
	case EEMPIRE_NAME::Dantari:
		empire_name = "Dantari Separatists";
		break;
	case EEMPIRE_NAME::Zolon:
		empire_name = "Zolon Empire";
		break;
	case EEMPIRE_NAME::Other:
		empire_name = "Other";
		break;
	case EEMPIRE_NAME::Pirate:
		empire_name = "Brotherhood of Iron";
		break;
	case EEMPIRE_NAME::Neutral:
		empire_name = "Neutral";
		break;
	case EEMPIRE_NAME::Unknown:
		empire_name = "Unknown";
		break;
	case EEMPIRE_NAME::Silessian:
		empire_name = "Silessian Confederacy";
		break;
	case EEMPIRE_NAME::Solus:
		empire_name = "Independent System of Solus";
		break;
	case EEMPIRE_NAME::Haiche:
		empire_name = "Haiche Protectorate";
		break;
	default:
		empire_name = "Unknown";
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

	static ConstructorHelpers::FObjectFinder<UDataTable> CampaignOOBDataTableObject(TEXT("DataTable'/Game/Game/DT_CampaignOOB.DT_CampaignOOB'"));

	if (CampaignOOBDataTableObject.Succeeded())
	{
		CampaignOOBDataTable = CampaignOOBDataTableObject.Object;
	}

	static ConstructorHelpers::FObjectFinder<UDataTable> CombatGroupDataTableObject(TEXT("DataTable'/Game/Game/DT_CombatGroup.DT_CombatGroup'"));

	if (CombatGroupDataTableObject.Succeeded())
	{
		CombatGroupDataTable = CombatGroupDataTableObject.Object;
	}

	static ConstructorHelpers::FObjectFinder<UDataTable> OrderOfBattleDataTableObject(TEXT("DataTable'/Game/Game/DT_OrderOfBattle.DT_OrderOfBattle'"));

	if (OrderOfBattleDataTableObject.Succeeded())
	{
		OrderOfBattleDataTable = OrderOfBattleDataTableObject.Object;
	}
	
	if (bClearTables) {
		CampaignDataTable->EmptyTable();
		CombatGroupDataTable->EmptyTable();
		OrderOfBattleDataTable->EmptyTable();
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

void USSWGameInstance::InitializeOperationsScreen(const FObjectInitializer& ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<UOperationsScreen> OperationsScreenWidget(TEXT("/Game/Screens/Operations/WB_Operations"));
	if (!ensure(OperationsScreenWidget.Class != nullptr))
	{
		return;
	}
	OperationsScreenWidgetClass = OperationsScreenWidget.Class;
}

void USSWGameInstance::InitializeMissionBriefingScreen(const FObjectInitializer& ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<UMissionLoading> MissionLoadingScreenWidget(TEXT("/Game/Screens/Mission/WB_MissionLoading"));
	if (!ensure(MissionLoadingScreenWidget.Class != nullptr))
	{
		return;
	}
	MissionLoadingWidgetClass = MissionLoadingScreenWidget.Class;
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

void USSWGameInstance::InitializeCampaignLoadingScreen(const FObjectInitializer& ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<UCampaignLoading> CampaignLoadingWidget(TEXT("/Game/Screens/Campaign/WB_CampaignLoading"));
	if (!ensure(CampaignLoadingWidget.Class != nullptr))
	{
		return;
	}
	CampaignLoadingWidgetClass = CampaignLoadingWidget.Class;
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

void USSWGameInstance::RemoveScreens()
{
	if (CampaignScreen) {
		//RemoveCampaignScreen();
	}
	if (CampaignLoading) {
		//RemoveCampaignLoadScreen();
	}
	if (OperationsScreen) {
		//RemoveOperationsScreen();
	}
	if (MainMenuDlg) {
		//RemoveMainMenuScreen();
	}
	if (MissionLoadingScreen) {
		//RemoveMissionBriefingScreen();
	}
}

void USSWGameInstance::OnGameTimerTick()
{
	SetGameTime(GetGameTime() + 1);
	SetCampaignTime(GetCampaignTime() + 1);
	UE_LOG(LogTemp, Log, TEXT("Campaign Timer: %d"), GetCampaignTime());
}

void USSWGameInstance::ShowMainMenuScreen()
{
	//MusicController->PlayMusic(MenuMusic); 
	RemoveScreens();

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

	if (UGameplayStatics::DoesSaveGameExist(PlayerSaveName, PlayerSaveSlot)) {
		ToggleFirstRunDlg(false);
	}
	else
	{
		ToggleFirstRunDlg(true);
	}
}

void USSWGameInstance::ShowCampaignScreen()
{
	//RemoveScreens();

	// Create widget
	//if(!CampaignScreen) {
		// Create widget
	CampaignScreen = CreateWidget<UCampaignScreen>(this, CampaignScreenWidgetClass);
	//}
	
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

void USSWGameInstance::ShowCampaignLoading()
{
	//RemoveScreens();

	// Create widget
	//if (!CampaignLoading) {
	CampaignLoading = CreateWidget<UCampaignLoading>(this, CampaignLoadingWidgetClass);
	//}
	// Add it to viewport
	CampaignLoading->AddToViewport(102);

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(CampaignLoading->TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}
	ToggleCampaignLoading(true);
}

void USSWGameInstance::ShowOperationsScreen()
{
	//RemoveScreens();

	// Create widget
	//if (!OperationsScreen) {
		// Create widget
	OperationsScreen = CreateWidget<UOperationsScreen>(this, OperationsScreenWidgetClass);
	//}

	// Add it to viewport
	OperationsScreen->AddToViewport(102);

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(OperationsScreen->TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}
	ToggleOperationsScreen(true);
}

void USSWGameInstance::ShowMissionBriefingScreen()
{
	//RemoveScreens();

	// Create widget
	//if (!MissionLoadingScreen) {
		// Create widget
	MissionLoadingScreen = CreateWidget<UMissionLoading>(this, MissionLoadingWidgetClass);
	//}

	// Add it to viewport
	MissionLoadingScreen->AddToViewport(101);

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(MissionLoadingScreen->TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}
	ToggleMissionBriefingScreen(true);
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
	//if (MainMenuDlg) {
	//	MainMenuDlg->EnableMenuButtons(!bVisible);
	//}
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

void USSWGameInstance::ToggleOperationsScreen(bool bVisible)
{
	if (OperationsScreen) {
		if (bVisible) {
			OperationsScreen->SetVisibility(ESlateVisibility::Visible);
		}
		else {
			OperationsScreen->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void USSWGameInstance::ToggleMissionBriefingScreen(bool bVisible)
{
	if (MissionLoadingScreen) {
		if (bVisible) {
			MissionLoadingScreen->SetVisibility(ESlateVisibility::Visible);
		}
		else {
			MissionLoadingScreen->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void USSWGameInstance::RemoveCampaignScreen()
{
	if (CampaignScreen) {
		CampaignScreen->RemoveFromParent();

		CampaignScreen = nullptr;
		if(GEngine) {
			GEngine->ForceGarbageCollection();
		}
	}
}

void USSWGameInstance::RemoveMainMenuScreen()
{
	if (MainMenuDlg) {
		MainMenuDlg->RemoveFromParent();

		MainMenuDlg = nullptr;
		if (GEngine) {
			GEngine->ForceGarbageCollection();
		}
	}
}

void USSWGameInstance::RemoveCampaignLoadScreen()
{
	if (CampaignLoading) {
		CampaignLoading->RemoveFromParent();

		CampaignLoading = nullptr;
		if (GEngine) {
			GEngine->ForceGarbageCollection();
		}
	}
}

void USSWGameInstance::RemoveOperationsScreen()
{
	if (OperationsScreen) {
		OperationsScreen->RemoveFromParent();

		OperationsScreen = nullptr;
		if (GEngine) {
			GEngine->ForceGarbageCollection();
		}
	}
}

void USSWGameInstance::RemoveMissionBriefingScreen()
{
	if (MissionLoadingScreen) {
		MissionLoadingScreen->RemoveFromParent();

		MissionLoadingScreen = nullptr;
		if (GEngine) {
			GEngine->ForceGarbageCollection();
		}
	}
}

void USSWGameInstance::ToggleCampaignLoading(bool bVisible)
{
	if (CampaignLoading) {
		if (bVisible) {
			CampaignLoading->SetVisibility(ESlateVisibility::Visible);
		}
		else {
			CampaignLoading->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void USSWGameInstance::SetGameMode(EMODE gm)
{

}

void USSWGameInstance::SetActiveCampaign(FS_Campaign campaign)
{
	ActiveCampaign = campaign;
}

void USSWGameInstance::SetActiveCampaignNr(int active)
{
	ActiveCampaignNr = active;
}

void USSWGameInstance::SetSelectedMissionNr(int active)
{
	SelectionMissionNr = active;
}

void USSWGameInstance::SetSelectedActionNr(int active)
{
	SelectionActionNr = active;
}

void USSWGameInstance::SetSelectedRosterNr(int active)
{
	SelectionRosterNr = active;
}

void USSWGameInstance::SetCampaignActive(bool bIsActive)
{
	bIsActiveCampaign = bIsActive;
}

FS_Campaign USSWGameInstance::GetActiveCampaign()
{
	return ActiveCampaign;
}

int USSWGameInstance::GetActiveCampaignNr()
{
	return ActiveCampaignNr;
}

int USSWGameInstance::GetSelectedMissionNr()
{
	return SelectionMissionNr;
}

int USSWGameInstance::GetSelectedActionNr()
{
	return SelectionActionNr;
}

int USSWGameInstance::GetSelectedRosterNr()
{
	return SelectionRosterNr;
}

bool USSWGameInstance::GetCampaignActive()
{
	return bIsActiveCampaign;
}

void USSWGameInstance::SetGameTime(int64 time)
{
	GameTime = time;
}

int64 USSWGameInstance::GetGameTime()
{
	return GameTime;
}

void USSWGameInstance::SetCampaignTime(int64 time)
{
	CampaignTime = time;
}

int64 USSWGameInstance::GetCampaignTime()
{
	return CampaignTime;
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

UTexture2D* USSWGameInstance::LoadPNGTextureFromFile(const FString& Path)
{
	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *Path)) {
		UE_LOG(LogTemp, Error, TEXT("Failed to load file: %s"), *Path);
		return nullptr;
	}

	// Get image wrapper for PNG
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	// Decode PNG
	if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(FileData.GetData(), FileData.Num())) {
		TArray<uint8> RawData;
		
		if (ImageWrapper->GetRaw(ERGBFormat::RGBA, 8, RawData)) {
			int32 Width = ImageWrapper->GetWidth();
			int32 Height = ImageWrapper->GetHeight();

			// Create the texture
			UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_R8G8B8A8);
			if (!Texture) return nullptr;

			// Lock and fill mip data
			void* TextureData = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
			FMemory::Memcpy(TextureData, RawData.GetData(), RawData.Num());
			Texture->GetPlatformData()->Mips[0].BulkData.Unlock();

			// Update texture
			Texture->UpdateResource();
			return Texture;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("Failed to decode PNG: %s"), *FilePath);
	return nullptr;
}

void USSWGameInstance::PlayMusic(USoundBase* Music)
{
	if (!Music)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayMusic: Invalid SoundBase"));
		return;
	}
	
	if (MusicController)
	{
		MusicController->PlayMusic(Music);
	}
}

void USSWGameInstance::StopMusic()
{
	if (MusicController) {
		MusicController->StopMusic();
	}
}

void USSWGameInstance::PlayMenuMusic()
{
	PlayMusic(MenuMusic);
}

void USSWGameInstance::PlaySoundFromFile(FString& AudioPath)
{
	USoundBase* Sound = Cast<USoundBase>(StaticLoadObject(USoundBase::StaticClass(), nullptr, *AudioPath));

	if (!Sound)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayUISound: Invalid SoundBase"));
		return;
	}

	if (MusicController)
	{
		MusicController->PlaySound(Sound);
	}
}

bool USSWGameInstance::IsSoundPlaying() {
	return MusicController->IsSoundPlaying();
}

void USSWGameInstance::InitializeAudioSystem()
{
	UWorld* World = GetWorld();
	if (World)
	{
		MusicController = World->SpawnActor<AMusicController>();
		UE_LOG(LogTemp, Log, TEXT("Music Controller Spawned"));
	}
}

void USSWGameInstance::ExitGame(UObject* Context) {
	UKismetSystemLibrary::QuitGame(Context, 0, EQuitPreference::Quit, true);
}

void USSWGameInstance::PlayUISound(UObject* Context, USoundBase* UISound)
{
	if (UISound)
	{
		UGameplayStatics::PlaySound2D(Context, UISound);
	}
}

void USSWGameInstance::PlayHoverSound(UObject* Context)
{
	PlayUISound(Context, HoverSound);
}

void USSWGameInstance::PlayAcceptSound(UObject* Context)
{
	PlayUISound(Context, AcceptSound);
}

FString USSWGameInstance::GetNameFromType(ECOMBATGROUP_TYPE nt)
{
	return EnumToDisplayString(nt);
}

FString USSWGameInstance::GetUnitFromType(ECOMBATUNIT_TYPE nt)
{
	return EnumToDisplayString(nt);
}

EEMPIRE_NAME USSWGameInstance::GetEmpireTypeFromIndex(int32 Index)
{
	UEnum* EnumPtr = StaticEnum<EEMPIRE_NAME>();
	if (EnumPtr && Index >= 0 && Index < EnumPtr->NumEnums())
	{
		int64 RawValue = EnumPtr->GetValueByIndex(Index);
		return static_cast<EEMPIRE_NAME>(RawValue);
	}

	// Optional: Print a warning if index is invalid
	UE_LOG(LogTemp, Warning, TEXT("Invalid index passed to GetEmpireTypeFromIndex: %d"), Index);

	// Return NONE if out of range
	return EEMPIRE_NAME::Unknown;
}

int32 USSWGameInstance::GetIndexFromEmpireType(EEMPIRE_NAME Type)
{
	UEnum* EnumPtr = StaticEnum<EEMPIRE_NAME>();
	int EmpireIndex = 8; 
	if (EnumPtr)
	{
		int64 Value = static_cast<int64>(Type);
		int32 Index = EnumPtr->GetIndexByValue(Value);
		if (Index != INDEX_NONE)
		{
			EmpireIndex = Index;
		}
	}
	return EmpireIndex;
}

FString USSWGameInstance::GetUnitPrefixFromType(ECOMBATUNIT_TYPE nt)
{
	FString Prefix;
	switch (nt) {
		case ECOMBATUNIT_TYPE::CRUISER: 
			Prefix = "CA-";
			break;
		case ECOMBATUNIT_TYPE::CARRIER:
			Prefix = "CV-";
			break;
		case ECOMBATUNIT_TYPE::FRIGATE:
			Prefix = "FF-";
			break;
		case ECOMBATUNIT_TYPE::DESTROYER:
			Prefix = "DD-";
			break;
		default:
			Prefix = "UNK-";
			break;
	} 
	return Prefix;
}

FString USSWGameInstance::GetEmpireTypeNameByIndex(int32 Index)
{
	UEnum* EnumPtr = StaticEnum<EEMPIRE_NAME>();
	if (!EnumPtr) return FString("Invalid");

	// Get value by index
	int64 EnumValue = EnumPtr->GetValueByIndex(Index);

	// Get display name from value
	return EnumPtr->GetDisplayNameTextByValue(EnumValue).ToString();
}

FString USSWGameInstance::GetEmpireDisplayName(EEMPIRE_NAME EnumValue)
{
	const UEnum* EnumPtr = StaticEnum<EEMPIRE_NAME>();
	if (!EnumPtr) return FString("Invalid");

	return EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(EnumValue)).ToString();
}


void USSWGameInstance::GetCampaignCombatant(int id, ECOMBATGROUP_TYPE Type) {
// Filters Table by Active in campaign
}

void USSWGameInstance::CreateOOBTable() {
	
	OrderOfBattleDataTable->EmptyTable();

	TArray<FS_OOBFleet> FleetArray;
	TArray<FS_OOBCarrier> CarrierArray;
	TArray<FS_OOBDestroyer> DestroyerArray;
	TArray<FS_OOBBattle> BattleArray;
	TArray<FS_OOBWing> WingArray;

	TArray<FS_OOBFighter> FighterArray;
	TArray<FS_OOBIntercept> InterceptorArray;
	TArray<FS_OOBAttack> AttackArray;
	TArray<FS_OOBLanding> LandingArray;

	TArray<FS_OOBBattalion> BattalionArray;
	TArray<FS_OOBBattery> BatteryArray;
	TArray<FS_OOBStation> StationArray;
	TArray<FS_OOBStarbase> StarbaseArray;

	TArray<FS_OOBCivilian> CivilianArray;
	TArray<FS_OOBMinefield> MinefieldArray;

	FS_OOBForce NewForce;
	FS_OOBForce* ForceRow;

	FS_OOBFleet CurrentFleet;
	
	int ForceId;
	int OldEmpire;
	int OldIff;

	int FleetId;
	FName RowName;

	for (FS_CombatGroup Item : CombatRosterData) // Make Initial Table
	{
		if (Item.Type == ECOMBATGROUP_TYPE::FORCE) {
			
			FleetArray.Empty();
			BattalionArray.Empty();
			CivilianArray.Empty();
			MinefieldArray.Empty();

			NewForce.Id = Item.Id;
			NewForce.Name = Item.DisplayName;
			NewForce.Iff = Item.Iff;
			NewForce.Location = Item.Region;
			NewForce.Empire = GetEmpireTypeFromIndex(Item.EmpireId);
			NewForce.Intel = Item.Intel;
			ForceId = Item.Id;
			OldIff = Item.Iff;
			OldEmpire = Item.EmpireId;
			RowName = FName(NewForce.Name);
			
			if (NewForce.Iff > -1)
			{
				OrderOfBattleDataTable->AddRow(FName(NewForce.Name), NewForce);
			}
		}
		else if (Item.Type == ECOMBATGROUP_TYPE::FLEET) {
			if(Item.ParentId == ForceId && Item.EmpireId == OldEmpire)
			{
				FS_OOBFleet NewFleet;
				
				NewFleet.Id = Item.Id;
				NewFleet.ParentId = Item.ParentId;
				NewFleet.Name = Item.DisplayName;
				NewFleet.Iff = Item.Iff;
				NewFleet.Location = Item.Region;
				NewFleet.Empire = GetEmpireTypeFromIndex(Item.EmpireId);
				NewFleet.Intel = Item.Intel;
				FleetId = Item.Id;
				FleetArray.Add(NewFleet);
			}
		}
		else if (Item.Type == ECOMBATGROUP_TYPE::STATION) {
			FS_OOBStation NewStation;

			NewStation.Id = Item.Id;
			NewStation.ParentId = Item.ParentId;
			NewStation.Name = Item.DisplayName;
			NewStation.Iff = Item.Iff;
			NewStation.Location = Item.Region;
			NewStation.Empire = GetEmpireTypeFromIndex(Item.EmpireId);
			NewStation.Intel = Item.Intel;
			NewStation.Id = Item.Id;
			StationArray.Add(NewStation);
		}
		else if (Item.Type == ECOMBATGROUP_TYPE::STARBASE) {
			FS_OOBStarbase NewStarbase;

			NewStarbase.Id = Item.Id;
			NewStarbase.ParentId = Item.ParentId;
			NewStarbase.Name = Item.DisplayName;
			NewStarbase.Iff = Item.Iff;
			NewStarbase.Location = Item.Region;
			NewStarbase.Empire = GetEmpireTypeFromIndex(Item.EmpireId);
			NewStarbase.Intel = Item.Intel;
			NewStarbase.Id = Item.Id;
			StarbaseArray.Add(NewStarbase);
		}
		else if (Item.Type == ECOMBATGROUP_TYPE::BATTERY) {
			FS_OOBBattery NewBattery;

			NewBattery.Id = Item.Id;
			NewBattery.ParentId = Item.ParentId;
			NewBattery.Name = Item.DisplayName;
			NewBattery.Iff = Item.Iff;
			NewBattery.Location = Item.Region;
			NewBattery.Empire = GetEmpireTypeFromIndex(Item.EmpireId);
			NewBattery.Intel = Item.Intel;
			NewBattery.Id = Item.Id;
			BatteryArray.Add(NewBattery);
		}
		else if (Item.Type == ECOMBATGROUP_TYPE::BATTALION) {
			if (Item.ParentId == ForceId && Item.EmpireId == OldEmpire)
			{
				FS_OOBBattalion NewBattalion;

				NewBattalion.Id = Item.Id;
				NewBattalion.ParentId = Item.ParentId;
				NewBattalion.Name = Item.DisplayName;
				NewBattalion.Iff = Item.Iff;
				NewBattalion.Location = Item.Region;
				NewBattalion.Empire = GetEmpireTypeFromIndex(Item.EmpireId);
				NewBattalion.Intel = Item.Intel;
				NewBattalion.Id = Item.Id;
				BattalionArray.Add(NewBattalion);
			}
		}
		else if (Item.Type == ECOMBATGROUP_TYPE::MINEFIELD) {
			if (Item.ParentId == ForceId && Item.EmpireId == OldEmpire)
			{
				FS_OOBMinefield NewMinefield;

				NewMinefield.Id = Item.Id;
				NewMinefield.ParentId = Item.ParentId;
				NewMinefield.Name = Item.DisplayName;
				NewMinefield.Iff = Item.Iff;
				NewMinefield.Location = Item.Region;
				NewMinefield.Empire = GetEmpireTypeFromIndex(Item.EmpireId);
				NewMinefield.Intel = Item.Intel;
				NewMinefield.Id = Item.Id;
				MinefieldArray.Add(NewMinefield);
			}
		}
		else if (Item.Type == ECOMBATGROUP_TYPE::CIVILIAN) {
			if (Item.ParentId == ForceId && Item.EmpireId == OldEmpire)
			{
				FS_OOBCivilian NewCivilian;

				NewCivilian.Id = Item.Id;
				NewCivilian.ParentId = Item.ParentId;
				NewCivilian.Name = Item.DisplayName;
				NewCivilian.Iff = Item.Iff;
				NewCivilian.Location = Item.Region;
				NewCivilian.Empire = GetEmpireTypeFromIndex(Item.EmpireId);
				NewCivilian.Intel = Item.Intel;
				NewCivilian.Id = Item.Id;
				CivilianArray.Add(NewCivilian);
			}
		}
		else if (Item.Type == ECOMBATGROUP_TYPE::CARRIER_GROUP) {
			FS_OOBCarrier NewCarrier;

			NewCarrier.Id = Item.Id;
			NewCarrier.ParentId = Item.ParentId;
			NewCarrier.Name = Item.DisplayName;
			NewCarrier.Iff = Item.Iff;
			NewCarrier.Location = Item.Region;
			NewCarrier.Empire = GetEmpireTypeFromIndex(Item.EmpireId);
			NewCarrier.Intel = Item.Intel;
			NewCarrier.Unit.SetNum(4);

			int32 Index = 0;
			for (const auto& UnitItem : Item.Unit)
			{
				if (!NewCarrier.Unit.IsValidIndex(Index))
					break;

				NewCarrier.Unit[Index].Name = UnitItem.UnitName;
				NewCarrier.Unit[Index].Count = 1;
				NewCarrier.Unit[Index].ParentId = Item.ParentId;
				NewCarrier.Unit[Index].Regnum = UnitItem.UnitRegnum;
				NewCarrier.Unit[Index].Empire = GetEmpireTypeFromIndex(Item.EmpireId);
				NewCarrier.Unit[Index].Location = Item.Region;
				NewCarrier.Unit[Index].ParentType = ECOMBATGROUP_TYPE::CARRIER_GROUP;

				if (UnitItem.UnitClass == "Cruiser") {
					NewCarrier.Unit[Index].Type = ECOMBATUNIT_TYPE::CRUISER;
				}
				else if (UnitItem.UnitClass == "Destroyer") {
					NewCarrier.Unit[Index].Type = ECOMBATUNIT_TYPE::DESTROYER;
				}
				else if (UnitItem.UnitClass == "Frigate") {
					NewCarrier.Unit[Index].Type = ECOMBATUNIT_TYPE::FRIGATE;
				}
				else if (UnitItem.UnitClass == "Carrier") {
					NewCarrier.Unit[Index].Type = ECOMBATUNIT_TYPE::CARRIER;
				}
				NewCarrier.Unit[Index].DisplayName = GetUnitPrefixFromType(NewCarrier.Unit[Index].Type) + UnitItem.UnitRegnum + " "+ UnitItem.UnitName;

				NewCarrier.Unit[Index].Design = UnitItem.UnitDesign;
				++Index;
			}

			CarrierArray.Add(NewCarrier);
		}
		else if (Item.Type == ECOMBATGROUP_TYPE::DESTROYER_SQUADRON) {
			FS_OOBDestroyer NewDestroyer;

			NewDestroyer.Id = Item.Id;
			NewDestroyer.ParentId = Item.ParentId;
			NewDestroyer.Name = Item.DisplayName;
			NewDestroyer.Iff = Item.Iff;
			NewDestroyer.Location = Item.Region;
			NewDestroyer.Empire = GetEmpireTypeFromIndex(Item.EmpireId);
			NewDestroyer.Intel = Item.Intel;
			NewDestroyer.Unit.SetNum(4);

			int32 Index = 0;
			for (const auto& UnitItem : Item.Unit)
			{
				if (!NewDestroyer.Unit.IsValidIndex(Index))
					break;

				NewDestroyer.Unit[Index].Name = UnitItem.UnitName;
				NewDestroyer.Unit[Index].Count = 1;
				NewDestroyer.Unit[Index].ParentId = Item.ParentId;
				NewDestroyer.Unit[Index].Empire = GetEmpireTypeFromIndex(Item.EmpireId);
				NewDestroyer.Unit[Index].Regnum = UnitItem.UnitRegnum;
				NewDestroyer.Unit[Index].Location = Item.Region;
				NewDestroyer.Unit[Index].ParentType = ECOMBATGROUP_TYPE::DESTROYER_SQUADRON;

				if (UnitItem.UnitClass == "Cruiser") {
					NewDestroyer.Unit[Index].Type = ECOMBATUNIT_TYPE::CRUISER;
				}
				else if (UnitItem.UnitClass == "Destroyer") {
					NewDestroyer.Unit[Index].Type = ECOMBATUNIT_TYPE::DESTROYER;
				}
				else if (UnitItem.UnitClass == "Frigate") {
					NewDestroyer.Unit[Index].Type = ECOMBATUNIT_TYPE::FRIGATE;
				}
				else if (UnitItem.UnitClass == "Carrier") {
					NewDestroyer.Unit[Index].Type = ECOMBATUNIT_TYPE::CARRIER;
				}

				NewDestroyer.Unit[Index].DisplayName = GetUnitPrefixFromType(NewDestroyer.Unit[Index].Type) + UnitItem.UnitRegnum + " " + UnitItem.UnitName;

				NewDestroyer.Unit[Index].Design = UnitItem.UnitDesign;
				++Index;
			}

			DestroyerArray.Add(NewDestroyer);
		}
		else if (Item.Type == ECOMBATGROUP_TYPE::BATTLE_GROUP) {
			FS_OOBBattle NewBattle;

			NewBattle.Id = Item.Id;
			NewBattle.ParentId = Item.ParentId;
			NewBattle.Name = Item.DisplayName;
			NewBattle.Iff = Item.Iff;
			NewBattle.Location = Item.Region;
			NewBattle.Empire = GetEmpireTypeFromIndex(Item.EmpireId);
			NewBattle.Intel = Item.Intel;
			NewBattle.Unit.SetNum(4);
			int32 Index = 0;
			for (const auto& UnitItem : Item.Unit)
			{
				if (!NewBattle.Unit.IsValidIndex(Index))
					break;

				NewBattle.Unit[Index].Name = UnitItem.UnitName;
				NewBattle.Unit[Index].Count = 1;
				NewBattle.Unit[Index].ParentId = Item.ParentId;
				NewBattle.Unit[Index].Regnum = UnitItem.UnitRegnum;
				NewBattle.Unit[Index].Empire = GetEmpireTypeFromIndex(Item.EmpireId);
				NewBattle.Unit[Index].Location = Item.Region;
				NewBattle.Unit[Index].ParentType = ECOMBATGROUP_TYPE::BATTLE_GROUP;

				if (UnitItem.UnitClass == "Cruiser") {
					NewBattle.Unit[Index].Type = ECOMBATUNIT_TYPE::CRUISER;
				}
				else if (UnitItem.UnitClass == "Destroyer") {
					NewBattle.Unit[Index].Type = ECOMBATUNIT_TYPE::DESTROYER;
				}
				else if (UnitItem.UnitClass == "Frigate") {
					NewBattle.Unit[Index].Type = ECOMBATUNIT_TYPE::FRIGATE;
				}
				else if (UnitItem.UnitClass == "Carrier") {
					NewBattle.Unit[Index].Type = ECOMBATUNIT_TYPE::CARRIER;
				}

				NewBattle.Unit[Index].DisplayName = GetUnitPrefixFromType(NewBattle.Unit[Index].Type) + UnitItem.UnitRegnum + " " + UnitItem.UnitName;

				NewBattle.Unit[Index].Design = UnitItem.UnitDesign;
				++Index;
			}

			BattleArray.Add(NewBattle);
		}
		else if (Item.Type == ECOMBATGROUP_TYPE::WING) {
			FS_OOBWing NewWing;

			NewWing.Id = Item.Id;
			NewWing.ParentId = Item.ParentId;
			NewWing.Name = Item.DisplayName;
			NewWing.Iff = Item.Iff;
			NewWing.Location = Item.Region;
			NewWing.Empire = GetEmpireTypeFromIndex(Item.EmpireId);
			NewWing.Intel = Item.Intel;
			WingArray.Add(NewWing);
		}

		else if (Item.Type == ECOMBATGROUP_TYPE::FIGHTER_SQUADRON) {
			FS_OOBFighter NewFighter;

			NewFighter.Id = Item.Id;
			NewFighter.ParentId = Item.ParentId;
			NewFighter.Name = Item.DisplayName;
			NewFighter.Iff = Item.Iff;
			NewFighter.Location = Item.Region;
			NewFighter.ParentType = Item.ParentType;
			NewFighter.Empire = GetEmpireTypeFromIndex(Item.EmpireId);
			NewFighter.Intel = Item.Intel;
			NewFighter.Unit.SetNum(1);

			int32 Index = 0;
			for (const auto& UnitItem : Item.Unit)
			{
				if (NewFighter.Unit.IsValidIndex(Index))
				{
					NewFighter.Unit[Index].Name = UnitItem.UnitName;
					NewFighter.Unit[Index].Count = UnitItem.UnitCount;
					NewFighter.Unit[Index].Location = Item.Region;
					NewFighter.Unit[Index].ParentId = Item.ParentId;
					NewFighter.Unit[Index].Empire = GetEmpireTypeFromIndex(Item.EmpireId);
					NewFighter.Unit[Index].Type = ECOMBATUNIT_TYPE::FIGHTER;
					NewFighter.Unit[Index].ParentType = ECOMBATGROUP_TYPE::FIGHTER_SQUADRON;
					NewFighter.Unit[Index].Design = UnitItem.UnitDesign;
				}
				++Index;
			}
			FighterArray.Add(NewFighter);
		}

		else if (Item.Type == ECOMBATGROUP_TYPE::INTERCEPT_SQUADRON) {
			FS_OOBIntercept NewIntercept;

			NewIntercept.Id = Item.Id;
			NewIntercept.ParentId = Item.ParentId;
			NewIntercept.Name = Item.DisplayName;
			NewIntercept.Iff = Item.Iff;
			NewIntercept.Location = Item.Region;
			NewIntercept.ParentType = Item.ParentType;
			NewIntercept.Empire = GetEmpireTypeFromIndex(Item.EmpireId);
			NewIntercept.Intel = Item.Intel;
			NewIntercept.Unit.SetNum(1);
			
			int32 Index = 0;
			for (const auto& UnitItem : Item.Unit)
			{
				if (NewIntercept.Unit.IsValidIndex(Index))
				{
					NewIntercept.Unit[Index].Name = UnitItem.UnitName;
					NewIntercept.Unit[Index].Count = UnitItem.UnitCount;
					NewIntercept.Unit[Index].Location = Item.Region;
					NewIntercept.Unit[Index].ParentId = Item.ParentId;
					NewIntercept.Unit[Index].Empire = GetEmpireTypeFromIndex(Item.EmpireId);
					NewIntercept.Unit[Index].Type = ECOMBATUNIT_TYPE::INTERCEPT;
					NewIntercept.Unit[Index].ParentType = ECOMBATGROUP_TYPE::INTERCEPT_SQUADRON;
					NewIntercept.Unit[Index].Design = UnitItem.UnitDesign;
				}
				++Index;
			}
			InterceptorArray.Add(NewIntercept);
		}

		else if (Item.Type == ECOMBATGROUP_TYPE::ATTACK_SQUADRON) {
			FS_OOBAttack NewAttack;

			NewAttack.Id = Item.Id;
			NewAttack.ParentId = Item.ParentId;
			NewAttack.Name = Item.DisplayName;
			NewAttack.Iff = Item.Iff;
			NewAttack.Location = Item.Region;
			NewAttack.ParentType = Item.ParentType;
			NewAttack.Empire = GetEmpireTypeFromIndex(Item.EmpireId);
			NewAttack.Intel = Item.Intel;
			NewAttack.Unit.SetNum(1);
			
			int32 Index = 0;
			for (const auto& UnitItem : Item.Unit)
			{
				if (NewAttack.Unit.IsValidIndex(Index))
				{
					NewAttack.Unit[Index].Name = UnitItem.UnitName;
					NewAttack.Unit[Index].Count = UnitItem.UnitCount;
					NewAttack.Unit[Index].Location = Item.Region;
					NewAttack.Unit[Index].ParentId = Item.ParentId;
					NewAttack.Unit[Index].Empire = GetEmpireTypeFromIndex(Item.EmpireId);
					NewAttack.Unit[Index].Type = ECOMBATUNIT_TYPE::ATTACK;
					NewAttack.Unit[Index].ParentType = ECOMBATGROUP_TYPE::ATTACK_SQUADRON;
					NewAttack.Unit[Index].Design = UnitItem.UnitDesign;
				}
				++Index;
			}
			AttackArray.Add(NewAttack);
		}

		else if (Item.Type == ECOMBATGROUP_TYPE::LCA_SQUADRON) {
			FS_OOBLanding NewLanding;

			NewLanding.Id = Item.Id;
			NewLanding.ParentId = Item.ParentId;
			NewLanding.Name = Item.DisplayName;
			NewLanding.Iff = Item.Iff;
			NewLanding.Location = Item.Region;
			NewLanding.ParentType = Item.ParentType;
			NewLanding.Empire = GetEmpireTypeFromIndex(Item.EmpireId);
			NewLanding.Intel = Item.Intel;
			NewLanding.Unit.SetNum(1);
			
			int32 Index = 0;
			for (const auto& UnitItem : Item.Unit)
			{
				if (NewLanding.Unit.IsValidIndex(Index))
				{
					NewLanding.Unit[Index].Name = UnitItem.UnitName;
					NewLanding.Unit[Index].Count = UnitItem.UnitCount;
					NewLanding.Unit[Index].Location = Item.Region;
					NewLanding.Unit[Index].ParentId = Item.ParentId;
					NewLanding.Unit[Index].Empire = GetEmpireTypeFromIndex(Item.EmpireId);
					NewLanding.Unit[Index].Type = ECOMBATUNIT_TYPE::LCA;
					NewLanding.Unit[Index].ParentType = ECOMBATGROUP_TYPE::LCA_SQUADRON;
					NewLanding.Unit[Index].Design = UnitItem.UnitDesign;
				}
				++Index;
			}
			LandingArray.Add(NewLanding);
		}
		// Loop through each fleet and assign its carriers
		for (FS_OOBBattalion& Battalion : BattalionArray)
		{
			Battalion.Battery.Empty(); // optional: clear old data
			Battalion.Station.Empty(); // optional: clear old data
			Battalion.Starbase.Empty(); // optional: clear old data

			for (const FS_OOBBattery& Battery : BatteryArray)
			{
				if (Battery.ParentId == Battalion.Id && Battery.Empire == Battalion.Empire)
				{
					Battalion.Battery.Add(Battery);
				}
			}
			for (const FS_OOBStation& Station : StationArray)
			{
				if (Station.ParentId == Battalion.Id && Station.Empire == Battalion.Empire)
				{
					Battalion.Station.Add(Station);
				}
			}
			for (const FS_OOBStarbase& Starbase : StarbaseArray)
			{
				if (Starbase.ParentId == Battalion.Id && Starbase.Empire == Battalion.Empire)
				{
					Battalion.Starbase.Add(Starbase);
				}
			}
		}

		// Loop through each fleet and assign its carriers
		for (FS_OOBFleet& Fleet : FleetArray)
		{
			Fleet.Carrier.Empty(); // optional: clear old data
			Fleet.Destroyer.Empty(); // optional: clear old data
			Fleet.Battle.Empty(); // optional: clear old data

			for (FS_OOBCarrier& Carrier : CarrierArray)
			{
				Carrier.Wing.Empty(); 
				Carrier.Fighter.Empty();
				Carrier.Attack.Empty();
				Carrier.Intercept.Empty();
				Carrier.Landing.Empty();

				if (Carrier.ParentId == Fleet.Id && Carrier.Empire == Fleet.Empire)
				{
					for (FS_OOBFighter& Fighter : FighterArray)
					{
						if (Fighter.ParentId == Carrier.Id && Fighter.Empire == Carrier.Empire && Fighter.ParentType == ECOMBATGROUP_TYPE::CARRIER_GROUP) {
							Carrier.Fighter.Add(Fighter);
						}
					}

					for (FS_OOBAttack& Attack : AttackArray)
					{
						if (Attack.ParentId == Carrier.Id && Attack.Empire == Carrier.Empire && Attack.ParentType == ECOMBATGROUP_TYPE::CARRIER_GROUP) {
							Carrier.Attack.Add(Attack);
						}
					}

					for (FS_OOBIntercept& Intercept : InterceptorArray)
					{
						if (Intercept.ParentId == Carrier.Id && Intercept.Empire == Carrier.Empire && Intercept.ParentType == ECOMBATGROUP_TYPE::CARRIER_GROUP) {
							Carrier.Intercept.Add(Intercept);
						}
					}
					for (FS_OOBLanding& Landing : LandingArray)
					{
						if (Landing.ParentId == Carrier.Id && Landing.Empire == Carrier.Empire && Landing.ParentType == ECOMBATGROUP_TYPE::CARRIER_GROUP) {
							Carrier.Landing.Add(Landing);
						}
					}
					for (FS_OOBWing& Wing : WingArray)
					{
						Wing.Fighter.Empty();
						Wing.Attack.Empty();
						Wing.Intercept.Empty();
						Wing.Landing.Empty();
						
						if (Wing.ParentId == Carrier.Id && Wing.Empire == Carrier.Empire) {
							
							for (FS_OOBFighter& Fighter : FighterArray)
							{
								if (Fighter.ParentId == Wing.Id && Fighter.Empire == Wing.Empire && Fighter.ParentType == ECOMBATGROUP_TYPE::WING) {
									Wing.Fighter.Add(Fighter);
								}
							}

							for (FS_OOBAttack& Attack : AttackArray)
							{
								if (Attack.ParentId == Wing.Id && Attack.Empire == Wing.Empire) {
									Wing.Attack.Add(Attack);
								}
							}

							for (FS_OOBIntercept& Intercept : InterceptorArray)
							{
								if (Intercept.ParentId == Wing.Id && Intercept.Empire == Wing.Empire) {
									Wing.Intercept.Add(Intercept);
								}
							}

							for (FS_OOBLanding& Landing : LandingArray)
							{
								if (Landing.ParentId == Wing.Id && Landing.Empire == Wing.Empire) {
									Wing.Landing.Add(Landing);
								}
							}
							Carrier.Wing.Add(Wing);
						}
					}
					Fleet.Carrier.Add(Carrier);
				}
			}
			
			for (const FS_OOBBattle& Battle : BattleArray)
			{
				if (Battle.ParentId == Fleet.Id && Battle.Empire == Fleet.Empire)
				{
					Fleet.Battle.Add(Battle);
				}
			}

			for (const FS_OOBDestroyer& Destroyer : DestroyerArray)
			{
				if (Destroyer.ParentId == Fleet.Id && Destroyer.Empire == Fleet.Empire)
				{
					Fleet.Destroyer.Add(Destroyer);
				}
			}
		}

		ForceRow = OrderOfBattleDataTable->FindRow<FS_OOBForce>(RowName, TEXT(""));

		if (ForceRow) {
			ForceRow->Fleet = FleetArray;
			ForceRow->Battalion = BattalionArray;
			ForceRow->Civilian = CivilianArray;
			ForceRow->Minefield = MinefieldArray;
		}
	}	
}

void USSWGameInstance::ExportDataTableToCSV(UDataTable* DataTable, const FString& FileName)
{
	if (!DataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("Export failed: DataTable is null."));
		return;
	}

	// You can use none or pretty names — make sure the flag is in scope
	const FString CSVData = DataTable->GetTableAsCSV(EDataTableExportFlags::None);

	const FString SavePath = FPaths::ProjectDir() + FileName;

	if (FFileHelper::SaveStringToFile(CSVData, *SavePath))
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully exported DataTable to: %s"), *SavePath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to write DataTable CSV to file."));
	}
}

TArray<FS_Combatant> USSWGameInstance::GetCombatantList()
{
	return CampaignData[PlayerInfo.Campaign].Combatant;
}

void USSWGameInstance::FlattenForce(const FS_OOBForce& Force, TArray<FS_OOBFlatEntry>& OutFlatList)
{
	int32 CurrentId = 0;
	RecursivelyFlattenForce(Force, INDEX_NONE, 0, CurrentId, OutFlatList);
}

void USSWGameInstance::RecursivelyFlattenForce(
	const FS_OOBForce& Force,
	int32 ParentId,
	int32 IndentLevel,
	int32& CurrentId,
	TArray<FS_OOBFlatEntry>& OutFlatList
)
{
	int32 ThisId = CurrentId++;

	FS_OOBFlatEntry ForceEntry;
	ForceEntry.Id = ThisId;
	ForceEntry.ParentId = ParentId;
	ForceEntry.DisplayName = Force.Name;
	ForceEntry.IndentLevel = IndentLevel;
	ForceEntry.GroupType = ECOMBATGROUP_TYPE::FORCE;

	OutFlatList.Add(ForceEntry);

	for (const FS_OOBFleet& Fleet : Force.Fleet)
	{
		RecursivelyFlattenFleet(Fleet, ThisId, IndentLevel + 1, CurrentId, OutFlatList);
	}
}

void USSWGameInstance::RecursivelyFlattenFleet(
	const FS_OOBFleet& Fleet,
	int32 ParentId,
	int32 IndentLevel,
	int32& CurrentId,
	TArray<FS_OOBFlatEntry>& OutFlatList
)
{
	int32 ThisId = CurrentId++;

	FS_OOBFlatEntry FleetEntry;
	FleetEntry.Id = ThisId;
	FleetEntry.ParentId = ParentId;
	FleetEntry.DisplayName = Fleet.Name;
	FleetEntry.IndentLevel = IndentLevel;
	FleetEntry.GroupType = ECOMBATGROUP_TYPE::FLEET;

	OutFlatList.Add(FleetEntry);

	for (const FS_OOBCarrier& Carrier : Fleet.Carrier)
	{
		RecursivelyFlattenCarrier(Carrier, ThisId, IndentLevel + 1, CurrentId, OutFlatList);
	}

	for (const FS_OOBDestroyer& Destroyer : Fleet.Destroyer)
	{
		RecursivelyFlattenDestroyer(Destroyer, ThisId, IndentLevel + 1, CurrentId, OutFlatList);
	}
}

void USSWGameInstance::RecursivelyFlattenCarrier(
	const FS_OOBCarrier& Carrier,
	int32 ParentId,
	int32 IndentLevel,
	int32& CurrentId,
	TArray<FS_OOBFlatEntry>& OutFlatList
)
{
	int32 ThisId = CurrentId++;

	FS_OOBFlatEntry Entry;
	Entry.Id = ThisId;
	Entry.ParentId = ParentId;
	Entry.DisplayName = Carrier.Name;
	Entry.IndentLevel = IndentLevel;
	Entry.GroupType = ECOMBATGROUP_TYPE::CARRIER_GROUP;

	OutFlatList.Add(Entry);

	for (const FS_OOBWing& Wing : Carrier.Wing)
	{
		RecursivelyFlattenWing(Wing, ThisId, IndentLevel + 1, CurrentId, OutFlatList);
	}
}

void USSWGameInstance::RecursivelyFlattenDestroyer(
	const FS_OOBDestroyer& Destroyer,
	int32 ParentId,
	int32 IndentLevel,
	int32& CurrentId,
	TArray<FS_OOBFlatEntry>& OutFlatList
)
{
	int32 ThisId = CurrentId++;

	FS_OOBFlatEntry Entry;
	Entry.Id = ThisId;
	Entry.ParentId = ParentId;
	Entry.DisplayName = Destroyer.Name;
	Entry.IndentLevel = IndentLevel;
	Entry.GroupType = ECOMBATGROUP_TYPE::DESTROYER_SQUADRON;

	OutFlatList.Add(Entry);
}

void USSWGameInstance::RecursivelyFlattenWing(
	const FS_OOBWing& Wing,
	int32 ParentId,
	int32 IndentLevel,
	int32& CurrentId,
	TArray<FS_OOBFlatEntry>& OutFlatList
)
{
	int32 ThisId = CurrentId++;

	FS_OOBFlatEntry Entry;
	Entry.Id = ThisId;
	Entry.ParentId = ParentId;
	Entry.DisplayName = Wing.Name;
	Entry.IndentLevel = IndentLevel;
	Entry.GroupType = ECOMBATGROUP_TYPE::WING;

	OutFlatList.Add(Entry);

	for (const FS_OOBFighter& Unit : Wing.Fighter)
	{
		RecursivelyFlattenFighter(Unit, ThisId, IndentLevel + 1, CurrentId, OutFlatList);
	}

	for (const FS_OOBAttack& Attack : Wing.Attack)
	{
		RecursivelyFlattenAttack(Attack, ThisId, IndentLevel + 1, CurrentId, OutFlatList);
	}

	for (const FS_OOBIntercept& Intercept : Wing.Intercept)
	{
		RecursivelyFlattenIntercept(Intercept, ThisId, IndentLevel + 1, CurrentId, OutFlatList);
	}

	for (const FS_OOBLanding& Landing : Wing.Landing)
	{
		RecursivelyFlattenLanding(Landing, ThisId, IndentLevel + 1, CurrentId, OutFlatList);
	}
}

void USSWGameInstance::RecursivelyFlattenFighter(
	const FS_OOBFighter& Unit,
	int32 ParentId,
	int32 IndentLevel,
	int32& CurrentId,
	TArray<FS_OOBFlatEntry>& OutFlatList
)
{
	int32 ThisId = CurrentId++;

	FS_OOBFlatEntry Entry;
	Entry.Id = ThisId;
	Entry.ParentId = ParentId;
	Entry.DisplayName = Unit.Name;
	Entry.IndentLevel = IndentLevel;
	Entry.GroupType = ECOMBATGROUP_TYPE::FIGHTER_SQUADRON;

	OutFlatList.Add(Entry);
}
void USSWGameInstance::RecursivelyFlattenAttack(
	const FS_OOBAttack& Attack,
	int32 ParentId,
	int32 IndentLevel,
	int32& CurrentId,
	TArray<FS_OOBFlatEntry>& OutFlatList
)
{
	int32 ThisId = CurrentId++;

	FS_OOBFlatEntry Entry;
	Entry.Id = ThisId;
	Entry.ParentId = ParentId;
	Entry.DisplayName = Attack.Name;
	Entry.IndentLevel = IndentLevel;
	Entry.GroupType = ECOMBATGROUP_TYPE::ATTACK_SQUADRON;

	OutFlatList.Add(Entry);
}

void USSWGameInstance::RecursivelyFlattenIntercept(
	const FS_OOBIntercept& Intercept,
	int32 ParentId,
	int32 IndentLevel,
	int32& CurrentId,
	TArray<FS_OOBFlatEntry>& OutFlatList
)
{
	int32 ThisId = CurrentId++;

	FS_OOBFlatEntry Entry;
	Entry.Id = ThisId;
	Entry.ParentId = ParentId;
	Entry.DisplayName = Intercept.Name;
	Entry.IndentLevel = IndentLevel;
	Entry.GroupType = ECOMBATGROUP_TYPE::INTERCEPT_SQUADRON;

	OutFlatList.Add(Entry);
}

void USSWGameInstance::RecursivelyFlattenLanding(
	const FS_OOBLanding& Landing,
	int32 ParentId,
	int32 IndentLevel,
	int32& CurrentId,
	TArray<FS_OOBFlatEntry>& OutFlatList
)
{
	int32 ThisId = CurrentId++;

	FS_OOBFlatEntry Entry;
	Entry.Id = ThisId;
	Entry.ParentId = ParentId;
	Entry.DisplayName = Landing.Name;
	Entry.IndentLevel = IndentLevel;
	Entry.GroupType = ECOMBATGROUP_TYPE::LCA_SQUADRON;

	OutFlatList.Add(Entry);
}
