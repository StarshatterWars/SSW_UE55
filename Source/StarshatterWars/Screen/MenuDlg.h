/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         MenuDlg.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Main Menu Screen
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "MenuDlg.generated.h"

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UMenuDlg : public UBaseScreen
{
	GENERATED_BODY()

public:	
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* btn_start;
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* btn_campaign;
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* btn_mission;
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* btn_player;
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* btn_multi;
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* btn_mod;
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* btn_tac;
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* btn_video;
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* btn_options;
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* btn_controls;
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* btn_quit;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* MenuTooltip;
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* GameVersion;

protected:	
	void NativeConstruct() override;

	UFUNCTION()
	void OnStartButtonClicked();
	UFUNCTION()
	void OnStartButtonHovered();
	UFUNCTION()
	void OnCampaignButtonClicked();
	UFUNCTION()
	void OnCampaignButtonHovered();

	UFUNCTION()
	void OnMissionButtonClicked();
	UFUNCTION()
	void OnPlayerButtonHovered();
	UFUNCTION()
	void OnMissionButtonHovered();
	UFUNCTION()
	void OnPlayerButtonClicked();
	UFUNCTION()
	void OnMultiplayerButtonClicked();
	UFUNCTION()
	void OnMultiplayerButtonHovered();
	UFUNCTION()
	void OnTacticalButtonClicked();
	UFUNCTION()
	void OnTacticalButtonHovered();
	UFUNCTION()
	void OnVideoButtonClicked();
	UFUNCTION()
	void OnOptionsButtonClicked();
	UFUNCTION()
	void OnOptionsButtonHovered();
	UFUNCTION()
	void OnControlsButtonClicked();
	UFUNCTION()
	void OnQuitButtonClicked();
	UFUNCTION()
	void OnQuitButtonHovered();
	UFUNCTION()
	void OnButtonUnHovered();
};
