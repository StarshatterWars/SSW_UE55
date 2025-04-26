// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/TimerHandle.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/DataTable.h"
#include "../Game/GameStructs.h"

#include "IImageWrapperModule.h"
#include "IImageWrapper.h"
#include "Modules/ModuleManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Engine/Texture2D.h"
#include "RenderUtils.h"
#include "Misc/DateTime.h"
#include "Misc/TimeSpan.h"

#include "Sound/SoundBase.h"

#include "Kismet/DataTableFunctionLibrary.h"
#include "SSWGameInstance.generated.h"


/**
 * 
 */

 class UUniverse;
 class USim;
 class AGalaxy;
 class AGameDataLoader;
 class AAwardInfoLoader;
 class AMusicController;
 class ACombatGroupLoader;
 class UQuitDlg;
 class UMenuDlg;
 class UFirstRun;
 class UCampaignScreen;
 class UOperationsScreen;
 class UCampaignLoading;
 class UMissionLoading;
 class DataLoader;
 class UPlayerSaveGame;


UCLASS()
class STARSHATTERWARS_API USSWGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	USSWGameInstance(
		const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Variables")
	FString ProjectPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Variables")
	FString FilePath;

	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	void SetProjectPath();

	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	void StartGameTimers();

	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	FString GetProjectPath();

	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	void Print(FString Msg, FString File);

	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	void SpawnGalaxy();

	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	void GetGameData();

	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	void StartGame();

	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	void LoadMainMenuScreen();

	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	void LoadTransitionScreen();

	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	void LoadOperationsScreen();
	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	void LoadMissionBriefingScreen();
	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	void LoadCampaignScreen();

	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	void LoadGameLevel(FString LevelName);

	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	void ShowMainMenuScreen();

	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	void ShowCampaignScreen();

	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	void ShowCampaignLoading();

	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	void ShowOperationsScreen();

	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	void ShowMissionBriefingScreen();

	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	void ShowQuitDlg();

	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	void ShowFirstRunDlg();

	UFUNCTION()
	void ToggleQuitDlg(bool bVisible);

	UFUNCTION()
	void ToggleFirstRunDlg(bool bVisible);

	UFUNCTION()
	void ToggleMenuButtons(bool bVisible);

	UFUNCTION()
	void ToggleCampaignScreen(bool bVisible);

	UFUNCTION()
	void ToggleOperationsScreen(bool bVisible);
	UFUNCTION()
	void ToggleMissionBriefingScreen(bool bVisible);

	UFUNCTION()
	void RemoveCampaignScreen();
	UFUNCTION()
	void RemoveMainMenuScreen();
	UFUNCTION()
	void RemoveCampaignLoadScreen();
	UFUNCTION()
	void RemoveOperationsScreen();
	UFUNCTION()
	void RemoveMissionBriefingScreen();

	UFUNCTION()
	void ToggleCampaignLoading(bool bVisible);

	UFUNCTION()
	void SetGameMode(EMODE gm);

	UFUNCTION()
	void SetActiveCampaign(FS_Campaign campaign);

	UFUNCTION()
	void SetActiveCampaignNr(int active);

	UFUNCTION()
	void SetSelectedMissionNr(int active);

	UFUNCTION()
	void SetSelectedActionNr(int active);
	UFUNCTION()
	void SetSelectedRosterNr(int active);
	UFUNCTION()
	void SetCampaignActive(bool bIsActive);

	UFUNCTION()
	FS_Campaign GetActiveCampaign();

	UFUNCTION()
	int GetActiveCampaignNr();
	UFUNCTION()
	int GetSelectedMissionNr();
	UFUNCTION()
	int GetSelectedActionNr();
	UFUNCTION()
	int GetSelectedRosterNr();
	UFUNCTION()
	bool GetCampaignActive();

	UFUNCTION()
	void SetGameTime(int64 time);

	UFUNCTION()
	int64 GetGameTime();

	UFUNCTION()
	void SetCampaignTime(int64 time);

	UFUNCTION()
	int64 GetCampaignTime();
	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	void SaveGame(FString SlotName, int32 UserIndex, FS_PlayerGameInfo PlayerInfo);

	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	void LoadGame(FString SlotName, int32 UserIndex);
	UFUNCTION(BlueprintCallable, Category = "Utilities")
	UTexture2D* LoadPNGTextureFromFile(const FString& Path);

	// Function to play music
    UFUNCTION()
    void PlayMusic(USoundBase* Music);

    // Function to stop music
    UFUNCTION()
    void StopMusic();

	// Function to play music
    UFUNCTION()
    void PlayMenuMusic();
	UFUNCTION()
	void PlaySoundFromFile(FString& AudioPath);
	UFUNCTION()
	bool IsSoundPlaying();
	UFUNCTION()
	void InitializeAudioSystem();

	UFUNCTION()
	void ExitGame(UObject* Context);
	UFUNCTION()
	void PlayUISound(UObject* Context, USoundBase* UISound);
	UFUNCTION()
	void PlayHoverSound(UObject* Context);
	UFUNCTION()
	void PlayAcceptSound(UObject* Context);
	UFUNCTION()
	FString GetNameFromType(ECOMBATGROUP_TYPE nt);
	FString GetUnitFromType(ECOMBATUNIT_TYPE nt);
	EEMPIRE_NAME GetEmpireTypeFromIndex(int32 Index);
	int32 GetIndexFromEmpireType(EEMPIRE_NAME Type);
	FString GetUnitPrefixFromType(ECOMBATUNIT_TYPE nt);
	FString GetEmpireTypeNameByIndex(int32 Index);
	FString GetEmpireDisplayName(EEMPIRE_NAME EnumValue);
	void GetCampaignCombatant(int id, ECOMBATGROUP_TYPE Type);
	void CreateCampaignOOBTable();
	void CreateOOBTable();
	void ExportDataTableToCSV(UDataTable* DataTable, const FString& FileName);
	
	void FlattenForce(const FS_OOBForce& ForceData, TArray<FS_OOBFlatEntry>& OutFlatList);

	AGalaxy* GameGalaxy;
	AGameDataLoader* GameData;
	AAwardInfoLoader* AwardData;
	ACombatGroupLoader* CombatGroupData;
	
	UPROPERTY()
	double StarDate;

	UPROPERTY()
	FString PlayerSaveName;

	UPROPERTY()
	int PlayerSaveSlot;
	
	UPROPERTY(EditAnywhere, Category = "UI Sound")
	USoundBase* AcceptSound;

	UPROPERTY(EditAnywhere, Category = "UI Sound")
	USoundBase* HoverSound;

	UPROPERTY(EditAnywhere, Category = "UI Sound")
	USoundBase* MenuMusic;

	DataLoader* loader;

	FString GetEmpireNameFromType(EEMPIRE_NAME emp);

	UMenuDlg* MainMenuDlg;
	UCampaignScreen* CampaignScreen;
	UOperationsScreen* OperationsScreen;
	UCampaignLoading* CampaignLoading;
	UMissionLoading* MissionLoadingScreen;
	UQuitDlg* QuitDlg;
	UFirstRun* FirstRunDlg;

	EMODE GameMode;

	class UDataTable* CampaignDataTable;
	class UDataTable* CombatGroupDataTable;
	class UDataTable* OrderOfBattleDataTable;
	class UDataTable* CampaignOOBDataTable;

	UPROPERTY(EditAnywhere)
	bool bClearTables;
	
	UPROPERTY()
	FS_PlayerGameInfo PlayerInfo;

	UPROPERTY()
	TArray<FS_Campaign> CampaignData;
	UPROPERTY()
	TArray<FS_CombatGroup> CombatRosterData;
	UPROPERTY()
	TArray<FS_CampaignMissionList> MissionList;
	UPROPERTY()
	TArray<FS_OOBForce> ForceList;

	UPROPERTY()
	bool MissionSelectionChanged;

	UPROPERTY()
	bool ActionSelectionChanged;

	UPROPERTY()
	bool RosterSelectionChanged;

	UPROPERTY()
	AMusicController* MusicController;
	
protected:
	virtual void Init() override;
	virtual void Shutdown() override;
	void ReadCampaignData();
	void ReadCombatRosterData();
	virtual bool InitContent();
	virtual bool InitGame();

	UPROPERTY()
	FString AppName;
	UPROPERTY()
	FString TitleText;
	UPROPERTY()
	FString PaletteName;

	// Internal variables for the state of the app
	UPROPERTY()
	bool              bIsWindowed;
	UPROPERTY()
	bool              bIsGameActive;
	UPROPERTY()
	bool              bIsDeviceLost;
	UPROPERTY()
	bool              bIsMinimized;
	UPROPERTY()
	bool              bIsMaximized;
	UPROPERTY()
	bool              bIgnoreSizeChange;
	UPROPERTY()
	bool              bIsDeviceInitialized;
	UPROPERTY()
	bool              bIsDeviceRestored;

	UPROPERTY()
	FS_Campaign       ActiveCampaign;

	UPROPERTY()
	int				  ActiveCampaignNr;

	UPROPERTY()
	bool			  bIsActiveCampaign;

	EGAMESTATUS Status;

	void InitializeDT(const FObjectInitializer& ObjectInitializer);

	private:
		UPROPERTY()
		bool bUniverseLoaded;

		UPROPERTY()
		int64 GameTime;

		UPROPERTY()
		int64 CampaignTime;

		UPROPERTY()
		int64 GameStart;
		
		UPROPERTY()
		int32 SelectionMissionNr;
		UPROPERTY()
		int32 SelectionActionNr;
		UPROPERTY()
		int32 SelectionRosterNr;

		void InitializeMainMenuScreen(const FObjectInitializer& ObjectInitializer);
		void InitializeCampaignScreen(const FObjectInitializer& ObjectInitializer);

		void InitializeCampaignLoadingScreen(const FObjectInitializer& ObjectInitializer);
		void InitializeOperationsScreen(const FObjectInitializer& ObjectInitializer);
		void InitializeMissionBriefingScreen(const FObjectInitializer& ObjectInitializer);

		void InitializeQuitDlg(const FObjectInitializer& ObjectInitializer);
		void InitializeFirstRunDlg(const FObjectInitializer& ObjectInitializer);
		
		UFUNCTION()
		void RemoveScreens();

		TSubclassOf<class UMenuDlg> MainMenuScreenWidgetClass;
		TSubclassOf<class UCampaignScreen> CampaignScreenWidgetClass;
		TSubclassOf<class UOperationsScreen> OperationsScreenWidgetClass;
		TSubclassOf<class UCampaignLoading> CampaignLoadingWidgetClass;
		TSubclassOf<class UMissionLoading> MissionLoadingWidgetClass;
		TSubclassOf<class UQuitDlg> QuitDlgWidgetClass;
		TSubclassOf<class UFirstRun> FirstRunDlgWidgetClass;

		FTimerHandle TimerHandle;

		UFUNCTION()
		void OnGameTimerTick();

		void RecursivelyFlattenForce(const FS_OOBForce& Force, int32 ParentId, int32 IndentLevel, int32& CurrentId, TArray<FS_OOBFlatEntry>& OutFlatList);
		void RecursivelyFlattenFleet(const FS_OOBFleet& Fleet, int32 ParentId, int32 IndentLevel, int32& CurrentId, TArray<FS_OOBFlatEntry>& OutFlatList);
		void RecursivelyFlattenBattle(const FS_OOBBattle& Battle, int32 ParentId, int32 IndentLevel, int32& CurrentId, TArray<FS_OOBFlatEntry>& OutFlatList);
		void RecursivelyFlattenCarrier(const FS_OOBCarrier& Carrier, int32 ParentId, int32 IndentLevel, int32& CurrentId, TArray<FS_OOBFlatEntry>& OutFlatList);
		void RecursivelyFlattenDestroyer(const FS_OOBDestroyer& Destroyer, int32 ParentId, int32 IndentLevel, int32& CurrentId, TArray<FS_OOBFlatEntry>& OutFlatList);
		void RecursivelyFlattenWing(const FS_OOBWing& Wing, int32 ParentId, int32 IndentLevel, int32& CurrentId, TArray<FS_OOBFlatEntry>& OutFlatList);
		void RecursivelyFlattenFighter(const FS_OOBFighter& Unit, int32 ParentId, int32 IndentLevel, int32& CurrentId, TArray<FS_OOBFlatEntry>& OutFlatList);
		void RecursivelyFlattenAttack(const FS_OOBAttack& Attack, int32 ParentId, int32 IndentLevel, int32& CurrentId, TArray<FS_OOBFlatEntry>& OutFlatList);
		void RecursivelyFlattenIntercept(const FS_OOBIntercept& Intercept, int32 ParentId, int32 IndentLevel, int32& CurrentId, TArray<FS_OOBFlatEntry>& OutFlatList);
		void RecursivelyFlattenLanding(const FS_OOBLanding& Landing, int32 ParentId, int32 IndentLevel, int32& CurrentId, TArray<FS_OOBFlatEntry>& OutFlatList);
};

