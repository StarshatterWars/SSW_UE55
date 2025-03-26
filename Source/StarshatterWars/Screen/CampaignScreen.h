// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "CampaignScreen.generated.h"

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UCampaignScreen : public UBaseScreen
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* Title;

	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* CancelButtonText;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* ApplyButtonText;
	
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* ApplyButton;
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* CancelButton;
	
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
};
