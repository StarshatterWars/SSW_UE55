/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         MenuDlg.cpp
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Main Menu Screen
*/


#include "MenuDlg.h"
#include "../System/SSWGameInstance.h"
#include "../System/Game.h"

void UMenuDlg::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UMenuDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UMenuDlg::EnableMenuButtons(bool bEnabled)
{
	btn_mission->SetIsEnabled(bEnabled);
	btn_multi->SetIsEnabled(bEnabled);
	btn_tac->SetIsEnabled(bEnabled);
	btn_player->SetIsEnabled(bEnabled);
}

void UMenuDlg::EnableStartMenuButton(bool bEnabled)
{
	btn_start->SetIsEnabled(bEnabled);
}

void UMenuDlg::NativeConstruct()
{
	Super::NativeConstruct();

	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();

	if (btn_start) {
		btn_start->OnClicked.AddDynamic(this, &UMenuDlg::OnStartButtonClicked);
		btn_start->OnHovered.AddDynamic(this, &UMenuDlg::OnStartButtonHovered);
		btn_start->OnUnhovered.AddDynamic(this, &UMenuDlg::OnButtonUnHovered);
		btn_start->SetIsEnabled(false);
	}
	if (btn_campaign) {
		btn_campaign->OnClicked.AddDynamic(this, &UMenuDlg::OnCampaignButtonClicked);
		btn_campaign->OnHovered.AddDynamic(this, &UMenuDlg::OnCampaignButtonHovered);
		btn_campaign->OnUnhovered.AddDynamic(this, &UMenuDlg::OnButtonUnHovered);
		
		if (UGameplayStatics::DoesSaveGameExist(SSWInstance->PlayerSaveName, SSWInstance->PlayerSaveSlot)) {
			btn_campaign->SetIsEnabled(true);
		}
		else
		{
			btn_campaign->SetIsEnabled(false);
		}
	}
	if (btn_mission) {
		btn_mission->OnClicked.AddDynamic(this, &UMenuDlg::OnMissionButtonClicked);
		btn_mission->OnHovered.AddDynamic(this, &UMenuDlg::OnMissionButtonHovered);
		btn_mission->OnUnhovered.AddDynamic(this, &UMenuDlg::OnButtonUnHovered);
		btn_mission->SetIsEnabled(false);
	}
	if (btn_player) {
		btn_player->OnClicked.AddDynamic(this, &UMenuDlg::OnPlayerButtonClicked);
		btn_player->OnHovered.AddDynamic(this, &UMenuDlg::OnPlayerButtonHovered);
		btn_player->OnUnhovered.AddDynamic(this, &UMenuDlg::OnButtonUnHovered);
		btn_player->SetIsEnabled(false);
	}
	if (btn_multi) {
		btn_multi->OnClicked.AddDynamic(this, &UMenuDlg::OnMultiplayerButtonClicked);
		btn_multi->OnHovered.AddDynamic(this, &UMenuDlg::OnMultiplayerButtonHovered);
		btn_multi->OnUnhovered.AddDynamic(this, &UMenuDlg::OnButtonUnHovered);
		btn_multi->SetIsEnabled(false);
	}
	if (btn_tac) {
		btn_tac->OnClicked.AddDynamic(this, &UMenuDlg::OnTacticalButtonClicked);
		btn_tac->OnHovered.AddDynamic(this, &UMenuDlg::OnTacticalButtonHovered);
		btn_tac->OnUnhovered.AddDynamic(this, &UMenuDlg::OnButtonUnHovered);
		btn_tac->SetIsEnabled(true);
	}
	if (btn_video) {
		btn_video->OnClicked.AddDynamic(this, &UMenuDlg::OnVideoButtonClicked);
		btn_video->SetIsEnabled(true);
	}
	if (btn_options) {
		btn_options->OnClicked.AddDynamic(this, &UMenuDlg::OnOptionsButtonClicked);
		btn_options->OnHovered.AddDynamic(this, &UMenuDlg::OnOptionsButtonHovered);
		btn_options->OnUnhovered.AddDynamic(this, &UMenuDlg::OnButtonUnHovered);
		btn_options->SetIsEnabled(true);
	}
	if (btn_controls) {
		btn_controls->OnClicked.AddDynamic(this, &UMenuDlg::OnControlsButtonClicked);
		btn_controls->SetIsEnabled(true);
	}
	if (btn_quit) {
		btn_quit->OnClicked.AddDynamic(this, &UMenuDlg::OnQuitButtonClicked);
		btn_quit->OnHovered.AddDynamic(this, &UMenuDlg::OnQuitButtonHovered);
		btn_quit->OnUnhovered.AddDynamic(this, &UMenuDlg::OnButtonUnHovered);
		btn_quit->SetIsEnabled(true);
	}

	if (MenuTooltip)
		MenuTooltip->SetText(FText::FromString(""));

	if(GameVersion)
		GameVersion->SetText(FText::FromString(Game::GetGameVersion()));

	if (SSWInstance->PlayerInfo.Campaign >= 0) {
		EnableMenuButtons(true);
		EnableStartMenuButton(true);
	} else {
		EnableMenuButtons(false);
		EnableStartMenuButton(false);
	}
}

void UMenuDlg::OnStartButtonClicked()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayAcceptSound(this);
	SSWInstance->LoadOperationsScreen();
}

void UMenuDlg::OnStartButtonHovered()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayHoverSound(this);

	if (MenuTooltip)
		MenuTooltip->SetText(FText::FromString("Start a new game, or resume your current game"));
}

void UMenuDlg::OnCampaignButtonClicked()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayAcceptSound(this);
	SSWInstance->ShowCampaignScreen();
}

void UMenuDlg::OnCampaignButtonHovered()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayHoverSound(this);

	if (MenuTooltip)
		MenuTooltip->SetText(FText::FromString("Start a new dynamic campaign, or load a saved game"));
}

void UMenuDlg::OnMissionButtonClicked()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayAcceptSound(this);
}

void UMenuDlg::OnMissionButtonHovered()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayHoverSound(this);

	if (MenuTooltip)
		MenuTooltip->SetText(FText::FromString("Play or create a scripted mission exercise"));
}

void UMenuDlg::OnPlayerButtonClicked()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayAcceptSound(this);
}

void UMenuDlg::OnPlayerButtonHovered()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayHoverSound(this);

	if (MenuTooltip)
		MenuTooltip->SetText(FText::FromString("Manage your logbook and player preferences"));
}

void UMenuDlg::OnMultiplayerButtonClicked()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayAcceptSound(this);
}

void UMenuDlg::OnMultiplayerButtonHovered()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayHoverSound(this);

	if (MenuTooltip)
		MenuTooltip->SetText(FText::FromString("Start or join a multiplayer scenario"));
}

void UMenuDlg::OnTacticalButtonClicked()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayAcceptSound(this);
}

void UMenuDlg::OnTacticalButtonHovered()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayHoverSound(this);

	if (MenuTooltip)
		MenuTooltip->SetText(FText::FromString("View ship and weapon stats and mission roles"));
}

void UMenuDlg::OnVideoButtonClicked()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayAcceptSound(this);
}

void UMenuDlg::OnOptionsButtonClicked()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayAcceptSound(this);
}

void UMenuDlg::OnOptionsButtonHovered()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayHoverSound(this);

	if (MenuTooltip)
		MenuTooltip->SetText(FText::FromString("Audio, Video, Gameplay, Control, and Mod configuration options"));
}

void UMenuDlg::OnControlsButtonClicked()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayAcceptSound(this);
}

void UMenuDlg::OnQuitButtonClicked()
{ 
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayAcceptSound(this);
	SSWInstance->ToggleQuitDlg(true);
}

void UMenuDlg::OnQuitButtonHovered()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayHoverSound(this);
	if (MenuTooltip)
		MenuTooltip->SetText(FText::FromString("Exit Starshatter and return to Windows"));
}

void UMenuDlg::OnButtonUnHovered()
{
	if (MenuTooltip)
		MenuTooltip->SetText(FText::FromString(""));
}

void UMenuDlg::ShowCampaignScreen()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->ShowCampaignScreen();
}







