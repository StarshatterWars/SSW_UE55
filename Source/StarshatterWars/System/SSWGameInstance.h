// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "UObject/UObjectGlobals.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/DataTable.h"
#include "../Game/GameStructs.h"

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
 class ACombatGroupLoader;
 class UQuitDlg;
 class UMenuDlg;
 class UFirstRun;
 class UCampaignScreen;
 class UOperationsScreen;
 class UCampaignLoading;
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
	FString GetProjectPath();

	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	void Print(FString Msg, FString File);

	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	void SpawnGalaxy();

	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	void GetCampaignData();

	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	void GetCombatRosterData();


	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	void GetAwardInfoData();


	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	void StartGame();

	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	void ShowMainMenuScreen();

	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	void ShowCampaignScreen();

	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	void ShowCampaignLoading();

	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	void ShowOperationsScreen();

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
	void RemoveCampaignScreen();
	UFUNCTION()
	void RemoveMainMenuScreen();
	UFUNCTION()
	void RemoveCampaignLoadScreen();
	UFUNCTION()
	void RemoveOperationsScreen();

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
	void SetCampaignActive(bool bIsActive);

	UFUNCTION()
	FS_Campaign GetActiveCampaign();

	UFUNCTION()
	int GetActiveCampaignNr();
	UFUNCTION()
	int GetSelectedMissionNr();
	
	UFUNCTION()
	bool GetCampaignActive();

	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	void SaveGame(FString SlotName, int32 UserIndex, FS_PlayerGameInfo PlayerInfo);

	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	void LoadGame(FString SlotName, int32 UserIndex);

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

	DataLoader* loader;

	FString GetEmpireNameFromType(EEMPIRE_NAME emp);

	UMenuDlg* MainMenuDlg;
	UCampaignScreen* CampaignScreen;
	UOperationsScreen* OperationsScreen;
	UCampaignLoading* CampaignLoading;
	UQuitDlg* QuitDlg;
	UFirstRun* FirstRunDlg;

	EMODE GameMode;

	class UDataTable* CampaignDataTable;

	UPROPERTY(EditAnywhere)
	bool bClearTables;
	
	UPROPERTY()
	FS_PlayerGameInfo PlayerInfo;

	UPROPERTY()
	TArray<FS_Campaign> CampaignData;
	UPROPERTY()
	TArray<FS_CampaignMissionList> MissionList;

	
	UPROPERTY()
	bool MissionSelectionChanged;
	
protected:
	virtual void Init() override;
	void ReadCampaignData();
	virtual void Shutdown() override;
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
		int32 SelectionMissionNr;

		void InitializeMainMenuScreen(const FObjectInitializer& ObjectInitializer);
		void InitializeCampaignScreen(const FObjectInitializer& ObjectInitializer);

		void InitializeCampaignLoadingScreen(const FObjectInitializer& ObjectInitializer);
		void InitializeOperationsScreen(const FObjectInitializer& ObjectInitializer);

		void InitializeQuitDlg(const FObjectInitializer& ObjectInitializer);
		void InitializeFirstRunDlg(const FObjectInitializer& ObjectInitializer);

		TSubclassOf<class UMenuDlg> MainMenuScreenWidgetClass;
		TSubclassOf<class UCampaignScreen> CampaignScreenWidgetClass;
		TSubclassOf<class UOperationsScreen> OperationsScreenWidgetClass;
		TSubclassOf<class UCampaignLoading> CampaignLoadingWidgetClass;
		TSubclassOf<class UQuitDlg> QuitDlgWidgetClass;
		TSubclassOf<class UFirstRun> FirstRunDlgWidgetClass;
};
