// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/GameStructs.h"

#include "Engine/Texture2D.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"
#include "Components/WidgetSwitcher.h"

#include "Components/ListView.h"

#include "Kismet/GameplayStatics.h"
#include "../System/SSWGameInstance.h"
#include "OperationsScreen.generated.h"

// Forward declarations
class UListView;
class UOOBForceItem;
class UOOBFleetItem;
class UOOBCarrierGroupItem;
class UOOBBattleItem;
class UOOBDestroyerItem;
class UOOBWingItem;
/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UOperationsScreen : public UUserWidget
{
	GENERATED_BODY()
	
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* TitleText;

	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* PlayerNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* GameTimeText;

	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* CampaignNameText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* DescriptionText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* SituationText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* Orders1Text;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* Orders2Text;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* Orders3Text;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* Orders4Text;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* LocationSystemText;

	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* MissionNameText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* MissionDescriptionText;

	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* MissionSitrepText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* MissionStartText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* MissionStatusText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* MissionTypeText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* MissionSystemText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* MissionRegionText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* MissionObjectiveText;

	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* IntelNameText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* IntelSourceText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* IntelLocationText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* IntelMessageText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* IntelDateText;

	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* GroupInfoText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* GroupLocationText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* GroupTypeText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* GroupEmpireText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* GroupInformationText;
	
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* OperationsModeText;
	
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* PlayButtonText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* CancelButtonText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* SelectButton;
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* CancelButton;

	UPROPERTY(meta = (BindWidgetOptional))
	class UImage* MissionImage;
	UPROPERTY(meta = (BindWidgetOptional))
	class UImage* IntelImage;
	UPROPERTY(meta = (BindWidgetOptional))
	class UImage* GroupImage;

	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* OrdersButton;
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* TheaterButton;
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* ForcesButton;
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* IntelButton;
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* MissionsButton;
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* AudioButton;

	UPROPERTY(meta = (BindWidgetOptional))
	class UBorder* FleetInfoBorder;

	UPROPERTY(meta = (BindWidgetOptional))
	class UBorder* CarrierInfoBorder;
	UPROPERTY(meta = (BindWidgetOptional))
	class UBorder* BattleInfoBorder;
	UPROPERTY(meta = (BindWidgetOptional))
	class UBorder* DesronInfoBorder;
	
	UPROPERTY(meta = (BindWidgetOptional))
	class UWidgetSwitcher* OperationalSwitcher;

	UPROPERTY(meta = (BindWidgetOptional))
	UListView* MissionList;

	UPROPERTY(meta = (BindWidgetOptional))
	UListView* IntelList;
	
	UPROPERTY(meta = (BindWidgetOptional))
	int SelectedMission;

	UPROPERTY()
	USoundBase* AudioAsset;

public:
	UFUNCTION()
	void SetSelectedMissionData(int Selected);
	UFUNCTION()
	void SetSelectedIntelData(int Selected);
	UFUNCTION()
	void SetSelectedRosterData(int Selected);

	UFUNCTION(BlueprintCallable)
	void BuildHierarchy();

	// Getter
	const TArray<FS_CombatGroup>& GetFlattenedList() const;

	// Setter
	void SetFlattenedList(const TArray<FS_CombatGroup>& NewList);

	// Getter
	const TArray<FS_CombatGroup>& GetAllGroupsList() const;

	// Setter
	void SetAllGroupsList(const TArray<FS_CombatGroup>& NewList);

	// Getter
	const TArray<FS_CombatGroup>& GetBaseGroupsList() const;

	void PrintGroupData(TArray<FS_CombatGroup> Group) const;
	
	// Setter
	void SetBaseGroupsList(const TArray<FS_CombatGroup>& NewList);

	void LoadForces();
	UPROPERTY(meta = (BindWidgetOptional))
	UListView* RosterView;

	UPROPERTY(meta = (BindWidgetOptional))
	UListView* ForceListView;

	UPROPERTY(meta = (BindWidgetOptional))
	UListView* FleetListView;

	UPROPERTY(meta = (BindWidgetOptional))
	UListView* CarrierListView;

	UPROPERTY(meta = (BindWidgetOptional))
	UListView* DesronListView;

	UPROPERTY(meta = (BindWidgetOptional))
	UListView* BattleListView;

	UPROPERTY(meta = (BindWidgetOptional))
	UListView* WingListView;

	UPROPERTY(meta = (BindWidgetOptional))
	UListView* SubGroupListView;

	UPROPERTY(EditAnywhere)
    TSubclassOf<UUserWidget> EntryWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FS_OOBForce> LoadedForces;

protected:
	void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UTexture2D* LoadTextureFromFile();
	FSlateBrush CreateBrushFromTexture(UTexture2D* Texture, FVector2D ImageSize);
	UFUNCTION()
	FString GetOrdinal(int id);
	UFUNCTION()
	void GetIntelImageFile(FString IntelImageName);
	UFUNCTION()
	void GetIntelAudioFile(FString IntelAudioName);
	UFUNCTION()
	void GetMissionImageFile(int selected);
	UFUNCTION()
	void OnSelectButtonClicked();
	UFUNCTION()
	void OnSelectButtonHovered();
	UFUNCTION()
	void OnSelectButtonUnHovered();
	UFUNCTION()
	void OnCancelButtonClicked();
	UFUNCTION()
	void OnCancelButtonHovered();
	UFUNCTION()
	void OnCancelButtonUnHovered();
	
	UFUNCTION()
	void OnOrdersButtonClicked();
	UFUNCTION()
	void OnOrdersButtonHovered();
	UFUNCTION()
	void OnOrdersButtonUnHovered();

	UFUNCTION()
	void OnTheaterButtonClicked();
	UFUNCTION()
	void OnTheaterButtonHovered();
	UFUNCTION()
	void OnTheaterButtonUnHovered();

	UFUNCTION()
	void OnForcesButtonClicked();
	UFUNCTION()
	void OnForcesButtonHovered();
	UFUNCTION()
	void OnForcesButtonUnHovered();

	UFUNCTION()
	void OnIntelButtonClicked();
	UFUNCTION()
	void OnIntelButtonHovered();
	UFUNCTION()
	void OnIntelButtonUnHovered();

	UFUNCTION()
	void OnMissionsButtonClicked();
	UFUNCTION()
	void OnMissionsButtonHovered();
	UFUNCTION()
	void OnMissionsButtonUnHovered();

	UFUNCTION()
	void OnAudioButtonClicked();
	UFUNCTION()
	void SetCampaignOrders();
	UFUNCTION()
	void SetCampaignMissions();
	UFUNCTION()
	void PopulateMissionList();
	UFUNCTION()
	void PopulateIntelList();

	UFUNCTION()
	void PopulateCombatRoster();
	UFUNCTION()
	FDateTime GetCampaignTime();

	void OnForceSelected(UObject* SelectedItem);
	void OnFleetSelected(UObject* SelectedItem);
	void OnCarrierSelected(UObject* SelectedItem);
	void OnDesronSelected(UObject* SelectedItem);
	void OnBattleGroupSelected(UObject* SelectedItem);

private:
	FS_Campaign ActiveCampaign;

	UPROPERTY()
	FString ImagePath;

	UPROPERTY()
	FString AudioPath;

	UPROPERTY()
	TArray<FS_CampaignAction> ActionList;

	UPROPERTY()
	TArray<FS_CombatGroup> RosterList;

	// Holds a flat version of the hierarchy (useful for displaying in a ListView)
	UPROPERTY()
	TArray<FS_CombatGroup> FlattenedList;
	UPROPERTY()
	TArray<FS_CombatGroup> AllGroups;
	UPROPERTY()
	TArray<FS_CombatGroup> RootGroups;
	
	int IndentLevel = 0;
};
