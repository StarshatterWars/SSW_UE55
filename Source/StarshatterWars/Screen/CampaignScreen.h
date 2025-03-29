// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "../Game/GameStructs.h"
#include "CampaignScreen.generated.h"

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UCampaignScreen : public UBaseScreen
{
	GENERATED_BODY()

	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* PlayerNameText;
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
	class UTextBlock* LocationRegionText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UComboBoxString* CampaignSelectDD;
	
protected:
	void NativeConstruct() override;

	UFUNCTION()
	void OnApplyButtonClicked();
	UFUNCTION()
	void OnApplyButtonHovered();
	UFUNCTION()
	void OnApplyButtonUnHovered();
	UFUNCTION()
	void OnCancelButtonClicked();
	UFUNCTION()
	void OnCancelButtonHovered();
	UFUNCTION()
	void OnCancelButtonUnHovered();
	
	UFUNCTION()
	void ReadCampaignData();

	UFUNCTION()
	void SetCampaignDDList();

	UFUNCTION()
	void SetSelectedData(int selected);

	UFUNCTION()
	void OnSetSelected(FString dropDownInt, ESelectInfo::Type type);
	
	TArray<FS_Campaign> CampaignData;

};
