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

void UMenuDlg::NativeConstruct()
{
	Super::NativeConstruct();

	if (btn_start) {
		btn_start->OnClicked.AddDynamic(this, &UMenuDlg::OnStartButtonClicked);
	}
	if (btn_campaign) {
		btn_campaign->OnClicked.AddDynamic(this, &UMenuDlg::OnCampaignButtonClicked);
	}
	if (btn_mission) {
		btn_mission->OnClicked.AddDynamic(this, &UMenuDlg::OnMissionButtonClicked);
	}
	if (btn_player) {
		btn_player->OnClicked.AddDynamic(this, &UMenuDlg::OnPlayerButtonClicked);
	}
	if (btn_multi) {
		btn_multi->OnClicked.AddDynamic(this, &UMenuDlg::OnMultiplayerButtonClicked);
	}
	if (btn_tac) {
		btn_tac->OnClicked.AddDynamic(this, &UMenuDlg::OnTacticalButtonClicked);
	}
	if (btn_video) {
		btn_video->OnClicked.AddDynamic(this, &UMenuDlg::OnVideoButtonClicked);
	}
	if (btn_options) {
		btn_options->OnClicked.AddDynamic(this, &UMenuDlg::OnOptionsButtonClicked);
	}
	if (btn_controls) {
		btn_controls->OnClicked.AddDynamic(this, &UMenuDlg::OnControlsButtonClicked);
	}
	if (btn_quit) {
		btn_quit->OnClicked.AddDynamic(this, &UMenuDlg::OnQuitButtonClicked);
	}
}

void UMenuDlg::OnStartButtonClicked()
{

}

void UMenuDlg::OnCampaignButtonClicked()
{

}

void UMenuDlg::OnMissionButtonClicked()
{

}

void UMenuDlg::OnPlayerButtonClicked()
{

}

void UMenuDlg::OnMultiplayerButtonClicked()
{

}

void UMenuDlg::OnTacticalButtonClicked()
{

}

void UMenuDlg::OnVideoButtonClicked()
{

}

void UMenuDlg::OnOptionsButtonClicked()
{

}

void UMenuDlg::OnControlsButtonClicked()
{

}

void UMenuDlg::OnQuitButtonClicked()
{

}





