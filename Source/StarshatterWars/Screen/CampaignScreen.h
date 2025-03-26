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
