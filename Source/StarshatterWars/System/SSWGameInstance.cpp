// Fill out your copyright notice in the Description page of Project Settings.


#include "SSWGameInstance.h"
#include "GameFramework/Actor.h"
#include "SimUniverse.h"
#include "Galaxy.h"
#include "DataLoader.h"
#include "Sim.h"
#include "GameDataLoader.h"

#include "MenuDlg.h"
#include "QuitDlg.h"
#include "FirstRun.h"
#include "CampaignScreen.h"
#include "OperationsScreen.h"
#include "MissionLoading.h"
#include "CampaignLoading.h"

#include "PlayerSaveGame.h"
#include "UniverseSaveGame.h" 
#include "Kismet/GameplayStatics.h"
#include "UObject/UObjectGlobals.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/Package.h" // For data asset support

#include "MusicController.h"
#include "MusicControllerInit.h"
#include "AudioDevice.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/TextureRenderTarget2D.h" 
#include "SystemOverview.h"

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

void USSWGameInstance::Print(const FString& A, const FString& B)
{
	const FString Msg = A + TEXT(" ") + B;

	UE_LOG(LogTemp, Log, TEXT("%s"), *Msg);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			5.0f,
			FColor::Cyan,
			Msg
		);
	}
}

void USSWGameInstance::SpawnGalaxy()
{
	/*UWorld* World = GetWorld();

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
	//	UE_LOG(LogTemp, Log, TEXT("World notxfound"));
	//}	*/	
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
	//	UE_LOG(LogTemp, Log, TEXT("World notxfound"));
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
	UniverseSaveSlotName = "Universe_Main";
	UniverseSaveUserIndex = 0;
	CampaignSaveSlotName = "Campaign";
	CampaignSaveIndex = 0;

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
	CreateOOBTable();
	SetupMusicController();
	//ExportDataTableToCSV(OrderOfBattleDataTable, TEXT("OOBExport.csv"));
	if (UTimerSubsystem* Timer = GetSubsystem<UTimerSubsystem>())
	{
		Timer->OnUniverseMinute.AddUObject(this, &USSWGameInstance::HandleUniverseMinuteAutosave);
		// optional: OnUniverseSecond for finer cadence, but minute is safer.
	}
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

void USSWGameInstance::SetActiveUnit(bool bShow, FString Name, EEMPIRE_NAME Empire, ECOMBATGROUP_TYPE Type, FString Loc)
{
	DisplayUnit.bShowUnit = bShow;
	DisplayUnit.Name = Name;
	DisplayUnit.Empire = Empire;
	DisplayUnit.Type = Type;
	DisplayUnit.Location = Loc;
}

void USSWGameInstance::SetActiveElement(bool bShow, FString Name, EEMPIRE_NAME Empire, ECOMBATUNIT_TYPE Type, FString Loc)
{
	DisplayElement.bShowUnit = bShow;
	DisplayElement.Name = Name;
	DisplayElement.Empire = Empire;
	DisplayElement.Type = Type;
	DisplayElement.Location = Loc;
}

void USSWGameInstance::SetActiveWidget(UUserWidget* Widget)
{
	ActiveWidget = Widget;
}

UUserWidget* USSWGameInstance::GetActiveWidget() {
	return ActiveWidget;
}

FS_DisplayUnit USSWGameInstance::GetActiveUnit()
{
	return DisplayUnit;
}

FS_DisplayElement USSWGameInstance::GetActiveElement()
{
	return DisplayElement;
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
	
	FString ContentProjectPath = FPaths::ProjectDir();
	ProjectPath.Append(TEXT("GameData/Content/"));

	loader->SetDataPath(TCHAR_TO_ANSI(*ContentProjectPath));
	//loader->ListFiles("content*", bundles);

	FString GameDataProjectPath = FPaths::ProjectDir(); 
	GameDataProjectPath = FPaths::ProjectDir();
	GameDataProjectPath.Append(TEXT("GameData/"));
	loader->SetDataPath(TCHAR_TO_ANSI(*GameDataProjectPath));

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

	static ConstructorHelpers::FObjectFinder<UDataTable> GalaxyDataTableObject(TEXT("DataTable'/Game/Game/DT_GalaxyMap.DT_GalaxyMap'"));

	if (GalaxyDataTableObject.Succeeded())
	{
		GalaxyDataTable = GalaxyDataTableObject.Object;
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
		GalaxyDataTable->EmptyTable();
	}
	GalaxyDataTable->EmptyTable();
	//OrderOfBattleDataTable->EmptyTable();
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

	if (AMusicController* Music = GetMusicController())
	{
		Music->PlayUISound(Sound);
	}
}

bool USSWGameInstance::IsSoundPlaying()
{
	if (AMusicController* MC = GetMusicController())
	{
		return MC->IsSoundPlaying();
	}
	return false;
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
	return EEMPIRE_NAME::Terellian;
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
	EEMPIRE_NAME OldEmpire;
	int OldIff;

	int FleetId;
	FName RowName;

	for (FS_CombatGroup Item : CombatRosterData) // Make Initial Table
	{
		if (Item.Type == ECOMBATGROUP_TYPE::FORCE) {
			
			FleetArray.Empty();
			BattalionArray.Empty();
			CivilianArray.Empty();

			NewForce.Id = Item.Id;
			NewForce.Name = Item.DisplayName;
			NewForce.Iff = Item.Iff;
			NewForce.Location = Item.Region;
			NewForce.Empire = Item.EmpireId;
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
				NewFleet.Empire = Item.EmpireId;
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
			NewStation.Empire = Item.EmpireId;
			NewStation.Intel = Item.Intel;
			NewStation.Id = Item.Id;
			NewStation.Unit.SetNum(1);

			int32 Index = 0;
			for (const auto& UnitItem : Item.Unit)
			{
				if (NewStation.Unit.IsValidIndex(Index))
				{
					NewStation.Unit[Index].Name = UnitItem.UnitName;
					NewStation.Unit[Index].Count = UnitItem.UnitCount;
					NewStation.Unit[Index].Location = Item.Region;
					NewStation.Unit[Index].ParentId = Item.ParentId;
					NewStation.Unit[Index].Empire = Item.EmpireId;
					NewStation.Unit[Index].Type = ECOMBATUNIT_TYPE::STATION;
					NewStation.Unit[Index].ParentType = ECOMBATGROUP_TYPE::STATION;
					NewStation.Unit[Index].Design = UnitItem.UnitDesign;
				}
				++Index;
			}
			StationArray.Add(NewStation);
		}
		else if (Item.Type == ECOMBATGROUP_TYPE::STARBASE) {
			FS_OOBStarbase NewStarbase;

			NewStarbase.Id = Item.Id;
			NewStarbase.ParentId = Item.ParentId;
			NewStarbase.Name = Item.DisplayName;
			NewStarbase.Iff = Item.Iff;
			NewStarbase.Location = Item.Region;
			NewStarbase.Empire = Item.EmpireId;
			NewStarbase.Intel = Item.Intel;
			NewStarbase.Id = Item.Id;
			NewStarbase.Unit.SetNum(1);

			int32 Index = 0;
			for (const auto& UnitItem : Item.Unit)
			{
				if (NewStarbase.Unit.IsValidIndex(Index))
				{
					NewStarbase.Unit[Index].Name = UnitItem.UnitName;
					NewStarbase.Unit[Index].Count = UnitItem.UnitCount;
					NewStarbase.Unit[Index].Location = Item.Region;
					NewStarbase.Unit[Index].ParentId = Item.ParentId;
					NewStarbase.Unit[Index].Empire = Item.EmpireId;
					NewStarbase.Unit[Index].Type = ECOMBATUNIT_TYPE::STARBASE;
					NewStarbase.Unit[Index].ParentType = ECOMBATGROUP_TYPE::STARBASE;
					NewStarbase.Unit[Index].Design = UnitItem.UnitDesign;
				}
				++Index;
			}
			StarbaseArray.Add(NewStarbase);
		}
		else if (Item.Type == ECOMBATGROUP_TYPE::BATTERY) {
			FS_OOBBattery NewBattery;

			NewBattery.Id = Item.Id;
			NewBattery.ParentId = Item.ParentId;
			NewBattery.Name = Item.DisplayName;
			NewBattery.Iff = Item.Iff;
			NewBattery.Location = Item.Region;
			NewBattery.Empire = Item.EmpireId;
			NewBattery.Intel = Item.Intel;
			NewBattery.Id = Item.Id;
			NewBattery.Unit.SetNum(1);

			int32 Index = 0;
			for (const auto& UnitItem : Item.Unit)
			{
				if (NewBattery.Unit.IsValidIndex(Index))
				{
					NewBattery.Unit[Index].Name = UnitItem.UnitName;
					NewBattery.Unit[Index].Count = UnitItem.UnitCount;
					NewBattery.Unit[Index].Location = Item.Region;
					NewBattery.Unit[Index].ParentId = Item.ParentId;
					NewBattery.Unit[Index].Empire = Item.EmpireId;
					NewBattery.Unit[Index].Type = ECOMBATUNIT_TYPE::BATTERY;
					NewBattery.Unit[Index].ParentType = ECOMBATGROUP_TYPE::BATTERY;
					NewBattery.Unit[Index].Design = UnitItem.UnitDesign;
				}
				++Index;
			}
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
				NewBattalion.Empire = Item.EmpireId;
				NewBattalion.Intel = Item.Intel;
				NewBattalion.Id = Item.Id;
				BattalionArray.Add(NewBattalion);
			}
		}
		else if (Item.Type == ECOMBATGROUP_TYPE::MINEFIELD) {
			FS_OOBMinefield NewMinefield;

			NewMinefield.Id = Item.Id;
			NewMinefield.ParentId = Item.ParentId;
			NewMinefield.Name = Item.DisplayName;
			NewMinefield.Iff = Item.Iff;
			NewMinefield.Location = Item.Region;
			NewMinefield.Empire = Item.EmpireId;
			NewMinefield.Intel = Item.Intel;
			NewMinefield.Id = Item.Id;
			NewMinefield.Unit.SetNum(1);

			int32 Index = 0;
			for (const auto& UnitItem : Item.Unit)
			{
				if (NewMinefield.Unit.IsValidIndex(Index))
				{
					NewMinefield.Unit[Index].Name = UnitItem.UnitName;
					NewMinefield.Unit[Index].Count = UnitItem.UnitCount;
					NewMinefield.Unit[Index].Location = Item.Region;
					NewMinefield.Unit[Index].ParentId = Item.ParentId;
					NewMinefield.Unit[Index].Empire = Item.EmpireId;
					NewMinefield.Unit[Index].Type = ECOMBATUNIT_TYPE::MINE;
					NewMinefield.Unit[Index].ParentType = ECOMBATGROUP_TYPE::MINEFIELD;
					NewMinefield.Unit[Index].Design = UnitItem.UnitDesign;
				}
				++Index;
			}
			MinefieldArray.Add(NewMinefield);
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
				NewCivilian.Empire = Item.EmpireId;
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
			NewCarrier.Empire = Item.EmpireId;
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
				NewCarrier.Unit[Index].Empire = Item.EmpireId;
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
			NewDestroyer.Empire = Item.EmpireId;
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
				NewDestroyer.Unit[Index].Empire = Item.EmpireId;
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
			NewBattle.Empire = Item.EmpireId;
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
				NewBattle.Unit[Index].Empire = Item.EmpireId;
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
			NewWing.Empire = Item.EmpireId;
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
			NewFighter.Empire = Item.EmpireId;
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
					NewFighter.Unit[Index].Empire = Item.EmpireId;
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
			NewIntercept.Empire = Item.EmpireId;
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
					NewIntercept.Unit[Index].Empire = Item.EmpireId;
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
			NewAttack.Empire = Item.EmpireId;
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
					NewAttack.Unit[Index].Empire = Item.EmpireId;
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
			NewLanding.Empire = Item.EmpireId;
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
					NewLanding.Unit[Index].Empire = Item.EmpireId;
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
			Fleet.Minefield.Empty(); // optional: clear old data

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

			for (const FS_OOBMinefield& Minefield : MinefieldArray)
			{
				if (Minefield.ParentId == Fleet.Id && Minefield.Empire == Fleet.Empire)
				{
					Fleet.Minefield.Add(Minefield);
				}
			}
		}

		ForceRow = OrderOfBattleDataTable->FindRow<FS_OOBForce>(RowName, TEXT(""));

		if (ForceRow) {
			ForceRow->Fleet = FleetArray;
			ForceRow->Battalion = BattalionArray;
			ForceRow->Civilian = CivilianArray;
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

void USSWGameInstance::SetupMusicController()
{
	UWorld* World = GetWorld();
	if (!World) return;

	for (TActorIterator<AMusicController> It(World); It; ++It)
	{
		return; // already exists
	}

	FVector Location = FVector(-1000, 0, 0);
	World->SpawnActor<AMusicController>(AMusicController::StaticClass(), Location, FRotator::ZeroRotator);
}

void USSWGameInstance::EnsureSystemOverview(UObject* InWorldContext, int32 Resolution)
{
	if (!InWorldContext)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnsureSystemOverview: InWorldContext is null"));
		return;
	}

	UWorld* World = InWorldContext->GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnsureSystemOverview: World is null"));
		return;
	}

	EnsureOverviewRT(Resolution);
	EnsureOverviewActor(World);

	if (OverviewActor && OverviewRT)
	{
		OverviewActor->SetRenderTarget(OverviewRT);
		OverviewActor->ConfigureCaptureForUI();
	}
}

void USSWGameInstance::EnsureOverviewRT(int32 Resolution)
{
	Resolution = FMath::Clamp(Resolution, 256, 4096);

	// If RT exists but wrong size, recreate it (optional).
	if (OverviewRT && OverviewRT.Get())
	{
		if (OverviewRT->SizeX == Resolution && OverviewRT->SizeY == Resolution)
		{
			return;
		}

		// Recreate to new size
		OverviewRT = nullptr;
	}

	// IMPORTANT: Outer = GameInstance, so it survives widget rebuilds and UI transitions
	OverviewRT = NewObject<UTextureRenderTarget2D>(this, TEXT("RT_SystemOverview"), RF_Transient);
	if (!OverviewRT)
	{
		UE_LOG(LogTemp, Error, TEXT("EnsureOverviewRT: Failed to allocate OverviewRT"));
		return;
	}

	OverviewRT->RenderTargetFormat = RTF_RGBA16f;
	OverviewRT->ClearColor = FLinearColor::Black;
	OverviewRT->bAutoGenerateMips = false;
	OverviewRT->InitAutoFormat(Resolution, Resolution);
	OverviewRT->UpdateResourceImmediate(true);

	UE_LOG(LogTemp, Log, TEXT("EnsureOverviewRT: Created OverviewRT %s (%dx%d) Outer=%s"),
		*GetNameSafe(OverviewRT.Get()), OverviewRT->SizeX, OverviewRT->SizeY, *GetNameSafe(this));
}

void USSWGameInstance::EnsureOverviewActor(UWorld* World)
{
	if (!World)
	{
		return;
	}

	// Actor is world-owned. GI holds a reference and respawns if level/world changes.
	if (OverviewActor && IsValid(OverviewActor))
	{
		if (OverviewActor->GetWorld() == World)
		{
			return;
		}

		// Different world (level travel): drop pointer so we respawn
		OverviewActor = nullptr;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.Name = TEXT("SystemOverviewActor");

	// Spawn far away so it's never seen in the playable scene
	const FVector SpawnLoc(1000000.f, 1000000.f, 1000000.f);
	const FRotator SpawnRot = FRotator::ZeroRotator;

	OverviewActor = World->SpawnActor<ASystemOverview>(ASystemOverview::StaticClass(), SpawnLoc, SpawnRot, Params);
	if (!OverviewActor)
	{
		UE_LOG(LogTemp, Error, TEXT("EnsureOverviewActor: Failed to spawn ASystemOverview"));
		return;
	}

	OverviewActor->SetActorHiddenInGame(true);
	OverviewActor->SetActorEnableCollision(false);

	UE_LOG(LogTemp, Log, TEXT("EnsureOverviewActor: Spawned %s in World=%s"),
		*GetNameSafe(OverviewActor), *GetNameSafe(World));
}

void USSWGameInstance::BuildAndCaptureSystemOverview(const TArray<FOverviewBody>& Bodies)
{
	if (!OverviewActor || !IsValid(OverviewActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("BuildAndCaptureSystemOverview: OverviewActor invalid"));
		return;
	}

	if (!OverviewRT || !IsValid(OverviewRT))
	{
		UE_LOG(LogTemp, Warning, TEXT("BuildAndCaptureSystemOverview: OverviewRT invalid"));
		return;
	}

	OverviewActor->SetRenderTarget(OverviewRT);
	OverviewActor->BuildDiorama(Bodies);
	OverviewActor->CaptureOnce();
}

void USSWGameInstance::DestroySystemOverview()
{
	if (OverviewActor && IsValid(OverviewActor))
	{
		OverviewActor->Destroy();
	}
	OverviewActor = nullptr;

	// Usually keep the RT alive, since you said GI should own it.
	// If you want to fully clean up, uncomment:
	// OverviewRT = nullptr;
}

void USSWGameInstance::RebuildSystemOverview(const FS_StarMap& StarMap)
{
	if (!OverviewActor)
	{
		return;
	}

	TArray<FOverviewBody> Bodies;
	Bodies.Reserve(1 + StarMap.Planet.Num() * 2); // rough reserve

	// Star (index 0)
	FOverviewBody Star;
	Star.Name = StarMap.Name;
	Star.ParentIndex = INDEX_NONE;
	Star.RadiusKm = StarMap.Radius;
	Star.OrbitKm = 0.f;
	Bodies.Add(Star);

	const int32 StarIndex = 0;

	// Planets
	for (const FS_PlanetMap& Planet : StarMap.Planet)
	{
		FOverviewBody PlanetBody;
		PlanetBody.Name = Planet.Name;
		PlanetBody.ParentIndex = StarIndex;
		PlanetBody.OrbitKm = Planet.Orbit;
		PlanetBody.RadiusKm = Planet.Radius;
		PlanetBody.OrbitAngleDeg = Planet.OrbitAngle;      // if you have it; otherwise random/cache
		PlanetBody.InclinationDeg = Planet.Inclination;    // if present
		const int32 ThisPlanetIndex = Bodies.Add(PlanetBody);

		// Moons (parent = this planet)
		for (const FS_MoonMap& Moon : Planet.Moon)
		{
			FOverviewBody MoonBody;
			MoonBody.Name = Moon.Name;
			MoonBody.ParentIndex = ThisPlanetIndex;
			MoonBody.OrbitKm = Moon.Orbit;
			MoonBody.RadiusKm = Moon.Radius;
			MoonBody.InclinationDeg = Moon.Inclination;
			MoonBody.OrbitAngleDeg = Moon.OrbitAngle;       // if you have it; otherwise random/cache
			Bodies.Add(MoonBody);
		}
	}

	OverviewActor->BuildDiorama(Bodies);
	OverviewActor->FrameDiorama();
	OverviewActor->CaptureOnce();
}

void USSWGameInstance::EnsureSystemOverview(
	UObject* Context,
	const FS_StarMap& StarMap,
	int32 Resolution)
{
	if (!Context) return;

	UWorld* World = Context->GetWorld();
	if (!World) return;

	EnsureOverviewRT(Resolution);
	EnsureOverviewActor(World);

	if (!OverviewActor || !OverviewRT) return;

	OverviewActor->SetRenderTarget(OverviewRT);
	OverviewActor->ConfigureCaptureForUI();

	// Only rebuild if system changed
	if (LastOverviewSystemName != StarMap.Name)
	{
		LastOverviewSystemName = StarMap.Name;
		RebuildSystemOverview(StarMap);
	}
}

//void USSWGameInstance::SetTimeScale(double NewTimeScale)
//{
//	TimeScale = FMath::Clamp(NewTimeScale, 0.0, 1.0e7);
//	UE_LOG(LogTemp, Warning, TEXT("TimeScale set to %.2f"), TimeScale);
//}

/*void USSWGameInstance::StartUniverseClock()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("StartUniverseClock: World is null"));
		return;
	}

	if (TimeStepSeconds <= 0.0)
	{
		TimeStepSeconds = 1.0;
	}

	FTimerManager& TM = World->GetTimerManager();

	// Prevent duplicates
	if (TM.IsTimerActive(UniverseTimerHandle))
	{
		TM.ClearTimer(UniverseTimerHandle);
	}

	TM.SetTimer(
		UniverseTimerHandle,
		this,
		&USSWGameInstance::OnUniverseClockTick,
		(float)TimeStepSeconds,
		true
	);

	UE_LOG(LogTemp, Log, TEXT("Universe clock started: Active=%s Step=%.3f TimeScale=%.2f"),
		TM.IsTimerActive(UniverseTimerHandle) ? TEXT("TRUE") : TEXT("FALSE"),
		TimeStepSeconds,
		TimeScale);
}

void USSWGameInstance::StopUniverseClock()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(UniverseTimerHandle);
	}

	UE_LOG(LogTemp, Log, TEXT("Universe clock stopped"));
}

void USSWGameInstance::OnUniverseClockTick()
{
	// Advance universe time
	const double DeltaUniverse = TimeStepSeconds * TimeScale;
	const uint64 AddSeconds = (uint64)FMath::Max(1.0, FMath::RoundToDouble(DeltaUniverse));
	UniverseTimeSeconds += AddSeconds;

	// ---------------------------------------------
	// PUB-SUB BROADCASTS
	// ---------------------------------------------

	// 1) Per-second broadcast
	if (UniverseTimeSeconds != LastBroadcastSecond)
	{
		LastBroadcastSecond = UniverseTimeSeconds;
		OnUniverseSecond.Broadcast(UniverseTimeSeconds);
	}

	// 2) Per-minute broadcast (Intel refresh)
	const uint64 CurrentMinute = UniverseTimeSeconds / 60ULL;
	if (CurrentMinute != LastBroadcastMinute)
	{
		LastBroadcastMinute = CurrentMinute;
		OnUniverseMinute.Broadcast(UniverseTimeSeconds);
	}

	// 3) Campaign T+ broadcast (only if campaign active)
	if (CampaignSave)
	{
		const uint64 TPlus = CampaignSave->GetTPlusSeconds(UniverseTimeSeconds);
		if (TPlus != LastBroadcastTPlus)
		{
			LastBroadcastTPlus = TPlus;
			OnCampaignTPlusChanged.Broadcast(UniverseTimeSeconds, TPlus);
		}
	}*/

	// ---------------------------------------------
	// Autosave / playtime logic (unchanged)
	// ---------------------------------------------
	//PlayerPlaytimeSeconds += (int64)TimeStepSeconds;

	//if (bUniverseAutosaveRequested)
	//{
	//	SaveUniverse();
		//bUniverseAutosaveRequested = false;
	//}
//}

/*void USSWGameInstance::HandlePostLoadMap(UWorld* LoadedWorld)
{
	// Timers belong to the loaded UWorld, so restart the clock after travel
	StartUniverseClock();

	UE_LOG(LogTemp, Log, TEXT("HandlePostLoadMap: restarted universe clock for World=%s"),
		*GetNameSafe(LoadedWorld));
}*/

void USSWGameInstance::SetUniverseSaveContext(const FString& SlotName, int32 UserIndex, UUniverseSaveGame* LoadedSave)
{
	UniverseSaveSlotName = SlotName;
	UniverseSaveUserIndex = UserIndex;
	CachedUniverseSave = LoadedSave;

	UE_LOG(LogTemp, Log, TEXT("Universe save context set: Slot=%s UserIndex=%d SaveObj=%s"),
		*UniverseSaveSlotName, UniverseSaveUserIndex, *GetNameSafe(CachedUniverseSave.Get()));
}

bool USSWGameInstance::SaveUniverse()
{
	const UTimerSubsystem* Timer = UGameInstance::GetSubsystem<UTimerSubsystem>();
	UE_LOG(LogTemp, Error, TEXT("in USSWGameInstance::SaveUniverse()"));

	// Guard: must have valid save context
	if (UniverseId.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("SaveUniverse: UniverseId is empty (notxloaded?)"));
		return false;
	}

	// Create a NEW object each time (no caching, no GC surprises)
	UUniverseSaveGame* SaveObj = Cast<UUniverseSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UUniverseSaveGame::StaticClass())
	);
	if (!SaveObj)
	{
		UE_LOG(LogTemp, Error, TEXT("SaveUniverse: CreateSaveGameObject failed"));
		return false;
	}

	SaveObj->UniverseId = UniverseId;
	SaveObj->UniverseSeed = UniverseSeed;
	SaveObj->UniverseBaseUnixSeconds = Timer->UniverseBaseUnixSeconds;
	SaveObj->UniverseTimeSeconds = Timer->UniverseTimeSeconds;

	// Use slot saving first (simplest + safest)
	const FString Slot = GetUniverseSlotName();   // MUST return a valid slot
	constexpr int32 UserIndex = 0;

	const bool bOK = UGameplayStatics::SaveGameToSlot(SaveObj, Slot, UserIndex);
	UE_LOG(LogTemp, Warning, TEXT("SaveUniverse: slot=%s ok=%d time=%llu"),
		*Slot, bOK ? 1 : 0, (unsigned long long)Timer->UniverseTimeSeconds);

	return bOK;
}

void USSWGameInstance::RequestUniverseAutosave()
{
	bUniverseAutosaveRequested = true;
}

#include "CampaignSave.h"
#include "Kismet/GameplayStatics.h"

bool USSWGameInstance::SaveCampaign()
{
	if (!CampaignSave)
	{
		UE_LOG(LogTemp, Warning, TEXT("SaveCampaign: No CampaignSave loaded"));
		return false;
	}

	if (CampaignSave->CampaignRowName.IsNone())
	{
		UE_LOG(LogTemp, Error, TEXT("SaveCampaign: CampaignRowName is None"));
		return false;
	}

	const FString Slot =
		UCampaignSave::MakeSlotNameFromRowName(CampaignSave->CampaignRowName);

	constexpr int32 UserIndex = 0;

	const bool bOK =
		UGameplayStatics::SaveGameToSlot(CampaignSave, Slot, UserIndex);

	UE_LOG(LogTemp, Warning,
		TEXT("SaveCampaign: slot=%s ok=%d row=%s"),
		*Slot,
		bOK ? 1 : 0,
		*CampaignSave->CampaignRowName.ToString());

	return bOK;
}

bool USSWGameInstance::SavePlayer(bool bForce)
{
	// call your existing SaveGame(PlayerSaveName, PlayerSaveSlot) here
	// return success/failure from your SaveGame implementation
	SaveGame(PlayerSaveName, PlayerSaveSlot, PlayerInfo);
	return true; // replace if your SaveGame returns a bool
}

UCampaignSave* USSWGameInstance::LoadOrCreateCampaignSave(int32 CampaignIndex, FName RowName, const FString& DisplayName)
{
	// Normalize
	CampaignIndex = FMath::Max(1, CampaignIndex);

	const FString Slot = UCampaignSave::MakeSlotNameFromRowName(RowName);
	constexpr int32 UserIndex = 0;

	UTimerSubsystem* Timer = GetSubsystem<UTimerSubsystem>();

	// Try load
	if (USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(Slot, UserIndex))
	{
		if (UCampaignSave* LoadedSave = Cast<UCampaignSave>(Loaded))
		{
			UE_LOG(LogTemp, Warning, TEXT("Campaign load attempt: Index=%d Slot=%s"),
				CampaignIndex, *Slot);

			UE_LOG(LogTemp, Warning, TEXT("Campaign load result: %s"),
				Loaded ? TEXT("SUCCESS") : TEXT("FAIL"));

			// Repair identity fields (optional)
			if (LoadedSave->CampaignIndex != CampaignIndex)
				LoadedSave->CampaignIndex = CampaignIndex;

			if (LoadedSave->CampaignRowName.IsNone() && !RowName.IsNone())
				LoadedSave->CampaignRowName = RowName;

			if (LoadedSave->CampaignDisplayName.IsEmpty() && !DisplayName.IsEmpty())
				LoadedSave->CampaignDisplayName = DisplayName;

			CampaignSave = LoadedSave;

			if (Timer)
			{
				// ---- One-time repair for older saves that never stored the anchor ----
				if (!CampaignSave->bInitialized || CampaignSave->CampaignStartUniverseSeconds == 0)
				{
					const uint64 Now = Timer->GetUniverseTimeSeconds();
					CampaignSave->InitializeCampaignClock(Now);

					// Persist the repaired anchor so it doesn't "reset" next load:
					UGameplayStatics::SaveGameToSlot(CampaignSave, Slot, UserIndex);
				}

				CampaignSave = LoadedSave;
				UE_LOG(LogTemp, Warning,
					TEXT("Loaded campaign from slot=%s  ObjName=%s  Row=%s  Index=%d  Start=%llu  Init=%d"),
					*Slot,
					*GetNameSafe(LoadedSave),
					*LoadedSave->CampaignRowName.ToString(),
					LoadedSave->CampaignIndex,
					(unsigned long long)LoadedSave->CampaignStartUniverseSeconds,
					LoadedSave->bInitialized ? 1 : 0);

				Timer->SetCampaignSave(LoadedSave);  // explicitly LoadedSave
			}

			return CampaignSave;
		}
	}

	// Create new
	UCampaignSave* NewSave = Cast<UCampaignSave>(
		UGameplayStatics::CreateSaveGameObject(UCampaignSave::StaticClass())
	);
	if (!NewSave)
	{
		UE_LOG(LogTemp, Error, TEXT("LoadOrCreateCampaignSave: Failed to create SaveGame object"));
		return nullptr;
	}

	NewSave->CampaignIndex = CampaignIndex;
	NewSave->CampaignRowName = RowName;
	NewSave->CampaignDisplayName = DisplayName;

	// Anchor campaign clock to CURRENT universe time (from subsystem)
	const uint64 NowUniverse = Timer ? Timer->GetUniverseTimeSeconds() : 0ULL;
	NewSave->InitializeCampaignClock(NowUniverse);

	// Persist
	if (!UGameplayStatics::SaveGameToSlot(NewSave, Slot, UserIndex))
	{
		UE_LOG(LogTemp, Error, TEXT("LoadOrCreateCampaignSave: SaveGameToSlot failed for %s"), *Slot);
	}

	// Assign + inject
	CampaignSave = NewSave;

	if (Timer)
	{
		Timer->SetCampaignSave(CampaignSave);
	}

	return CampaignSave;
}

UCampaignSave* USSWGameInstance::CreateNewCampaignSave(int32 CampaignIndex, FName RowName, const FString& DisplayName)
{
	// Normalize
	CampaignIndex = FMath::Max(1, CampaignIndex);

	if (RowName.IsNone())
	{
		UE_LOG(LogTemp, Error, TEXT("CreateNewCampaignSave: RowName is None"));
		return nullptr;
	}

	const FString Slot = UCampaignSave::MakeSlotNameFromRowName(RowName);
	constexpr int32 UserIndex = 0;

	UTimerSubsystem* Timer = GetSubsystem<UTimerSubsystem>();

	// Create new save object
	UCampaignSave* NewSave = Cast<UCampaignSave>(
		UGameplayStatics::CreateSaveGameObject(UCampaignSave::StaticClass())
	);

	if (!NewSave)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateNewCampaignSave: Failed to create SaveGame object"));
		return nullptr;
	}

	// Identity
	NewSave->CampaignIndex = CampaignIndex;
	NewSave->CampaignRowName = RowName;
	NewSave->CampaignDisplayName = DisplayName;

	// Reset campaign timeline by anchoring to current universe time
	const uint64 NowUniverse = Timer ? Timer->GetUniverseTimeSeconds() : 0ULL;
	NewSave->InitializeCampaignClock(NowUniverse);

	// Persist (overwrites existing slot for this campaign row)
	const bool bOK = UGameplayStatics::SaveGameToSlot(NewSave, Slot, UserIndex);

	UE_LOG(LogTemp, Warning,
		TEXT("CreateNewCampaignSave: slot=%s ok=%d row=%s index=%d start=%llu"),
		*Slot,
		bOK ? 1 : 0,
		*RowName.ToString(),
		CampaignIndex,
		(unsigned long long)NewSave->CampaignStartUniverseSeconds);

	// Assign + inject into timer subsystem so UI starts at T+ 00:00:00
	CampaignSave = NewSave;

	if (Timer)
	{
		Timer->SetCampaignSave(NewSave);
	}

	return NewSave;
}

UCampaignSave* USSWGameInstance::LoadOrCreateSelectedCampaignSave()
{
	return LoadOrCreateCampaignSave(SelectedCampaignIndex, SelectedCampaignRowName, SelectedCampaignDisplayName);
}

void USSWGameInstance::EnsureCampaignSaveLoaded()
{
	// If already loaded, nothing to do
	if (CampaignSave)
		return;

	int32 UiIndex = PlayerInfo.Campaign;
	if (UiIndex < 0)
	{
		UiIndex = 0;
		PlayerInfo.Campaign = 0;
	}

	SelectedCampaignRowName = NAME_None;
	SelectedCampaignDisplayName = TEXT("");

	if (CampaignDataTable)
	{
		TArray<FName> RowNames = CampaignDataTable->GetRowNames();

		struct FTempCampaignRef
		{
			int32 Index0 = 0;
			FName RowName = NAME_None;
			FString Name;
			bool bAvailable = false;
		};

		TArray<FTempCampaignRef> Sorted;
		Sorted.Reserve(RowNames.Num());

		for (const FName& RN : RowNames)
		{
			const FS_Campaign* Row = CampaignDataTable->FindRow<FS_Campaign>(RN, TEXT("EnsureCampaignSaveLoaded"));
			if (!Row) continue;

			FTempCampaignRef Ref;
			Ref.Index0 = Row->Index;
			Ref.RowName = RN;
			Ref.Name = Row->Name;
			Ref.bAvailable = Row->bAvailable;
			Sorted.Add(Ref);
		}

		Sorted.Sort([](const FTempCampaignRef& A, const FTempCampaignRef& B)
			{
				return A.Index0 < B.Index0;
			});

		UiIndex = FMath::Clamp(UiIndex, 0, Sorted.Num() - 1);

		if (Sorted.IsValidIndex(UiIndex))
		{
			SelectedCampaignRowName = Sorted[UiIndex].RowName;
			SelectedCampaignDisplayName = Sorted[UiIndex].Name;
			SelectedCampaignIndex = Sorted[UiIndex].Index0 + 1;
		}
	}
	else
	{
		if (CampaignData.Num() > 0)
		{
			UiIndex = FMath::Clamp(UiIndex, 0, CampaignData.Num() - 1);
			SelectedCampaignDisplayName = CampaignData[UiIndex].Name;
			SelectedCampaignIndex = UiIndex + 1;
		}
	}

	if (SelectedCampaignIndex < 1)
	{
		ShowCampaignScreen();
		return;
	}

	// ---- Load/Create the campaign save ----
	LoadOrCreateSelectedCampaignSave();

	// If load failed, do notxproceed to Operations (avoid crash)
	if (!CampaignSave)
	{
		UE_LOG(LogTemp, Error, TEXT("EnsureCampaignSaveLoaded: Failed to load/create CampaignSave"));
		ShowCampaignScreen();
		return;
	}

	// ---------------------------------------------------------
	// NEW: Inject the loaded campaign save into the TimerSubsystem
	// ---------------------------------------------------------
	if (UTimerSubsystem* Timer = GetSubsystem<UTimerSubsystem>())
	{
		Timer->SetCampaignSave(CampaignSave);
	}
}

AMusicController* USSWGameInstance::GetMusicController()
{
	if (MusicController && IsValid(MusicController))
	{
		return MusicController;
	}

	// Try to find an existing one in the world
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AMusicController> It(World); It; ++It)
		{
			MusicController = *It;
			return MusicController;
		}
	}

	// Optional: spawn if you have a class to spawn
	// If you don’t have one, leave it null and just guard calls.
	return nullptr;
}

FString USSWGameInstance::GetCampaignTPlusString() const
{
	const UTimerSubsystem* Timer = UGameInstance::GetSubsystem<UTimerSubsystem>();
	if (CampaignSave)
	{
		// Uses CampaignSave anchor + UniverseTimeSeconds
		return CampaignSave->GetTPlusDisplay(Timer->UniverseTimeSeconds);
	}

	// notxloaded yet
	return TEXT("T+ --/--:--:--");
}

FString USSWGameInstance::GetCampaignAndUniverseTimeLine() const
{
	const UTimerSubsystem* Timer = GetSubsystem<UTimerSubsystem>();

	const FString UniverseStr = Timer
		? Timer->GetUniverseDateTimeString()
		: TEXT("--");

	const FString TPlusStr = GetCampaignTPlusString(); // your existing method, still fine

	return FString::Printf(
		TEXT("UNIVERSE: %s   |   T+ %s"),
		*UniverseStr,
		*TPlusStr
	);
}

void USSWGameInstance::HandleUniverseMinuteAutosave(uint64 UniverseSecondsNow)
{
	// Universe autosave (your existing method)
	SaveUniverse();

	// Campaign autosave (only if you actually have mutable campaign state)
	SaveCampaign();
}