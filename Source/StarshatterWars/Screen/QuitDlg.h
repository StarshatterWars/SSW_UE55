// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "QuitDlg.generated.h"

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UQuitDlg : public UBaseScreen
{
	GENERATED_BODY()
	
	public:
		UPROPERTY(meta = (BindWidgetOptional))
		class UButton* btn_apply;
		UPROPERTY(meta = (BindWidgetOptional))
		class UButton* btn_cancel;
		UPROPERTY(meta = (BindWidgetOptional))
		class UTextBlock* ExitTitle;
		; UPROPERTY(meta = (BindWidgetOptional))
		class UTextBlock* ExitPrompt;
		
	protected:
		void NativeConstruct() override;

		UFUNCTION()
		void OnApplyClicked();
		UFUNCTION()
		void OnCancelClicked();
};
