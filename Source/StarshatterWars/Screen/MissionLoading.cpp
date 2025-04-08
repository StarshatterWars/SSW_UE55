// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "MissionLoading.h"
#include "../Foundation/FormatUtil.h"

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

	if (SituationButton) {
		SituationButton->OnClicked.AddDynamic(this, &UMissionLoading::OnSituationButtonClicked);
		SituationButton->OnHovered.AddDynamic(this, &UMissionLoading::OnSituationButtonHovered);
		SituationButton->OnUnhovered.AddDynamic(this, &UMissionLoading::OnSituationButtonUnHovered);
	}

	if (PackageButton) {
		PackageButton->OnClicked.AddDynamic(this, &UMissionLoading::OnPackageButtonClicked);
		PackageButton->OnHovered.AddDynamic(this, &UMissionLoading::OnPackageButtonHovered);
		PackageButton->OnUnhovered.AddDynamic(this, &UMissionLoading::OnPackageButtonUnHovered);
	}

	if (MapButton) {
		MapButton->OnClicked.AddDynamic(this, &UMissionLoading::OnMapButtonClicked);
		MapButton->OnHovered.AddDynamic(this, &UMissionLoading::OnMapButtonHovered);
		MapButton->OnUnhovered.AddDynamic(this, &UMissionLoading::OnMapButtonUnHovered);
	}

	if (WeaponsButton) {
		WeaponsButton->OnClicked.AddDynamic(this, &UMissionLoading::OnWeaponsButtonClicked);
		WeaponsButton->OnHovered.AddDynamic(this, &UMissionLoading::OnWeaponsButtonHovered);
		WeaponsButton->OnUnhovered.AddDynamic(this, &UMissionLoading::OnWeaponsButtonUnHovered);
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
		MissionStartTimeText->SetText(FText::FromString(FormatDayFromString(SSWInstance->GetActiveCampaign().MissionList[SSWInstance->PlayerInfo.Mission].Start)));
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

void UMissionLoading::OnSituationButtonClicked()
{
	if (MissionScreenSwitcher) {
		MissionScreenSwitcher->SetActiveWidgetIndex(0);
	}
}

void UMissionLoading::OnSituationButtonHovered()
{
}

void UMissionLoading::OnSituationButtonUnHovered()
{
}

void UMissionLoading::OnPackageButtonClicked()
{
	if (MissionScreenSwitcher) {
		MissionScreenSwitcher->SetActiveWidgetIndex(1);
	}
}

void UMissionLoading::OnPackageButtonHovered()
{
}

void UMissionLoading::OnPackageButtonUnHovered()
{
}

void UMissionLoading::OnMapButtonClicked()
{
	if (MissionScreenSwitcher) {
		MissionScreenSwitcher->SetActiveWidgetIndex(2);
	}
}

void UMissionLoading::OnMapButtonHovered()
{
}

void UMissionLoading::OnMapButtonUnHovered()
{
}

void UMissionLoading::OnWeaponsButtonClicked()
{
	if (MissionScreenSwitcher) {
		MissionScreenSwitcher->SetActiveWidgetIndex(3);
	}
}

void UMissionLoading::OnWeaponsButtonHovered()
{
}

void UMissionLoading::OnWeaponsButtonUnHovered()
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
