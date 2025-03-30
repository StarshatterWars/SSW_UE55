// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OperationsScreen.h"

void UOperationsScreen::NativeConstruct()
{
	Super::NativeConstruct();

	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->LoadGame(SSWInstance->PlayerSaveName, SSWInstance->PlayerSaveSlot);

	if (Title)
		Title->SetText(FText::FromString("Operational Command"));
	if (CancelButton) {
		CancelButton->OnClicked.AddDynamic(this, &UOperationsScreen::OnCancelButtonClicked);
		CancelButton->OnHovered.AddDynamic(this, &UOperationsScreen::OnCancelButtonHovered);
		CancelButton->OnUnhovered.AddDynamic(this, &UOperationsScreen::OnCancelButtonUnHovered);
		if (CancelButtonText) {
			CancelButtonText->SetText(FText::FromString("CANCEL"));
		}
	}
	if (SelectButton) {
		SelectButton->OnClicked.AddDynamic(this, &UOperationsScreen::OnSelectButtonClicked);
		SelectButton->OnHovered.AddDynamic(this, &UOperationsScreen::OnSelectButtonHovered);
		SelectButton->OnUnhovered.AddDynamic(this, &UOperationsScreen::OnSelectButtonUnHovered);
	}
	if (PlayerNameText) {
		PlayerNameText->SetText(FText::FromString(SSWInstance->PlayerInfo.Name));
	}
	ActiveCampaign = SSWInstance->GetActiveCampaign();
	
	SetSelectedData();
}

void UOperationsScreen::OnSelectButtonClicked()
{
}

void UOperationsScreen::OnSelectButtonHovered()
{
}

void UOperationsScreen::OnSelectButtonUnHovered()
{
}


void UOperationsScreen::OnCancelButtonClicked()
{

	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->ToggleOperationsScreen(false);
	SSWInstance->ShowMainMenuScreen();

}

void UOperationsScreen::OnCancelButtonHovered()
{
}

void UOperationsScreen::OnCancelButtonUnHovered()
{
}

void UOperationsScreen::SetSelectedData()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();

	if (CampaignNameText) {
		CampaignNameText->SetText(FText::FromString(SSWInstance->GetActiveCampaign().Name));
	}

	if (DescriptionText) {
		DescriptionText->SetText(FText::FromString(SSWInstance->GetActiveCampaign().Description));
	}

	if (SituationText) {
		SituationText->SetText(FText::FromString(SSWInstance->GetActiveCampaign().Situation));
	}

	if (Orders1Text) {

		Orders1Text->SetText(FText::FromString(SSWInstance->GetActiveCampaign().Orders[0]));
	}
	if (Orders2Text) {

		Orders2Text->SetText(FText::FromString(SSWInstance->GetActiveCampaign().Orders[1]));
	}
	if (Orders3Text) {

		Orders3Text->SetText(FText::FromString(SSWInstance->GetActiveCampaign().Orders[2]));
	}
	if (Orders4Text) {

		Orders4Text->SetText(FText::FromString(SSWInstance->GetActiveCampaign().Orders[3]));
	}
	if (LocationSystemText) {
		FString LocationSystem = SSWInstance->GetActiveCampaign().System + "/" + SSWInstance->GetActiveCampaign().Region;
		LocationSystemText->SetText(FText::FromString(LocationSystem));
	}
}
