// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "../Game/GameStructs.h"
#include "FirstRun.generated.h"

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UFirstRun : public UBaseScreen
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* btn_apply;
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* btn_cancel;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* FirstRunTitle;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* FirstRunPrompt;

	UPROPERTY(meta = (BindWidgetOptional))
	class UEditableTextBox* PlayerNameBox;

protected:
	void NativeConstruct() override;

	UFUNCTION()
	void OnApplyClicked();
	UFUNCTION()
	void OnCancelClicked();
	
	UPROPERTY()
	FS_PlayerGameInfo PlayerData;

};
