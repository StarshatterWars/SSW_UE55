// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "MissionLoading.h"

void UMissionLoading::GetSelectedMissionData()
{
	USSWGameInstance * SSWInstance = (USSWGameInstance*)GetGameInstance();
	SelectedMission = SSWInstance->PlayerInfo.Mission;
}

void UMissionLoading::NativeConstruct()
{
	Super::NativeConstruct();

	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->LoadGame(SSWInstance->PlayerSaveName, SSWInstance->PlayerSaveSlot);

	if (TitleText)
		TitleText->SetText(FText::FromString("Mission Briefing").ToUpper());

	if (CancelButton) {
		CancelButton->OnClicked.AddDynamic(this, &UMissionLoading::OnCancelButtonClicked);
		CancelButton->OnHovered.AddDynamic(this, &UMissionLoading::OnCancelButtonHovered);
		CancelButton->OnUnhovered.AddDynamic(this, &UMissionLoading::OnCancelButtonUnHovered);
	}

	if (CancelButtonText) {
		CancelButtonText->SetText(FText::FromString("BACK"));
	}

	if (SelectButton) {
		SelectButton->OnClicked.AddDynamic(this, &UMissionLoading::OnSelectButtonClicked);
		SelectButton->OnHovered.AddDynamic(this, &UMissionLoading::OnSelectButtonHovered);
		SelectButton->OnUnhovered.AddDynamic(this, &UMissionLoading::OnSelectButtonUnHovered);
	}

	if (PlayButtonText) {
		PlayButtonText->SetText(FText::FromString("PLAY"));
	}

	if (MissionScreenSwitcher) {
		MissionScreenSwitcher->SetActiveWidgetIndex(0);
	}

	if(CampaignNameText) {
		CampaignNameText->SetText(FText::FromString(SSWInstance->GetActiveCampaign().Name));
	}
	if (MissionNameText) {
		MissionNameText->SetText(FText::FromString(SSWInstance->GetActiveCampaign().MissionList[SSWInstance->PlayerInfo.Mission].Name));
	}

	if (SystemLocationText) {
		SystemLocationText->SetText(FText::FromString(SSWInstance->GetActiveCampaign().MissionList[SSWInstance->PlayerInfo.Mission].System));
	}

	if (RegionLocationText) {
		RegionLocationText->SetText(FText::FromString(SSWInstance->GetActiveCampaign().MissionList[SSWInstance->PlayerInfo.Mission].Region));
	}
	if (MissionStartTimeText) {
		MissionStartTimeText->SetText(FText::FromString(SSWInstance->GetActiveCampaign().MissionList[SSWInstance->PlayerInfo.Mission].Start));
	}

	if (PlayerNameText) {
		PlayerNameText->SetText(FText::FromString(SSWInstance->PlayerInfo.Name));
	}
}

void UMissionLoading::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
}

void UMissionLoading::OnSelectButtonClicked()
{
}

void UMissionLoading::OnSelectButtonHovered()
{
}

void UMissionLoading::OnSelectButtonUnHovered()
{
}

void UMissionLoading::OnCancelButtonClicked()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->LoadOperationsScreen();
}

void UMissionLoading::OnCancelButtonHovered()
{
}

void UMissionLoading::OnCancelButtonUnHovered()
{
}
