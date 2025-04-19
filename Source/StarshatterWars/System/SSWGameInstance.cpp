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
#include "../Screen/OperationsScreen.h"
#include "../Screen/MissionLoading.h"
#include "../Screen/CampaignLoading.h"

#include "../Game/PlayerSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UObjectGlobals.h"
#include "../Foundation/MusicController.h"
#include "AudioDevice.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

#undef UpdateResource
#undef PlaySound

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
	bClearTables = false;

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

	if (bClearTables) {
		CampaignDataTable->EmptyTable();
	}
	if (UGameplayStatics::DoesSaveGameExist(PlayerSaveName, PlayerSaveSlot)) {
		LoadGame(PlayerSaveName, PlayerSaveSlot);
		UE_LOG(LogTemp, Log, TEXT("Player Name: %s"), *PlayerInfo.Name);

		if (PlayerInfo.Campaign >= 0) {
			ReadCampaignData();
		}
	}
	//InitializeAudioSystem();
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
			UE_LOG(LogTemp, Log, TEXT("Campaign Available: %s"), (Row->Available ? TEXT("true") : TEXT("false")));
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

	if(bClearTables)
		CampaignDataTable->EmptyTable();
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
	//RemoveScreens();

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

