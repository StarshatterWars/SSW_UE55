// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "Kismet/GameplayStatics.h"
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
		UPROPERTY(EditAnywhere, Category = "UI Sound")
		USoundBase* HoverSound;

		UPROPERTY(EditAnywhere, Category = "UI Sound")
		USoundBase* AcceptSound;
		
	protected:
		void NativeConstruct() override;
		
		UFUNCTION()
		void PlayUISound(UObject* WorldContext, USoundBase* UISound);
		UFUNCTION()
		void OnApplyClicked();
		UFUNCTION()
		void OnCancelClicked();
};
