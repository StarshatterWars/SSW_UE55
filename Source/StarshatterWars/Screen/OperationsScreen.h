// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/GameStructs.h"

#include "Engine/Texture2D.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"
#include "Components/WidgetSwitcher.h"
#include "Components/ListView.h"

#include "Kismet/GameplayStatics.h"
#include "../System/SSWGameInstance.h"
#include "OperationsScreen.generated.h"

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
	class UWidgetSwitcher* OperationalSwitcher;

	UPROPERTY(meta = (BindWidgetOptional))
	UListView* MissionList;

	UPROPERTY(meta = (BindWidgetOptional))
	UListView* IntelList;

	UPROPERTY(EditAnywhere, Category = "UI Sound")
	USoundBase* HoverSound;

	UPROPERTY(EditAnywhere, Category = "UI Sound")
	USoundBase* AcceptSound;


	UPROPERTY(meta = (BindWidgetOptional))
	int SelectedMission;

public:
	UFUNCTION()
	void SetSelectedMissionData(int Selected);

protected:
	void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION()
	void PlayUISound(UObject* WorldContext, USoundBase* UISound);

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
	void SetCampaignOrders();
	UFUNCTION()
	void SetCampaignMissions();
	UFUNCTION()
	void PopulateMissionList();
	UFUNCTION()
	void PopulateIntelList();
	
	UFUNCTION()
	FDateTime GetCampaignTime();
		

private:
	FS_Campaign ActiveCampaign;
};
