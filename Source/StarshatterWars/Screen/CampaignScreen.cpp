// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "CampaignScreen.h"

void UCampaignScreen::NativeConstruct()
{
	Super::NativeConstruct();
	
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();

	if (Title)
		Title->SetText(FText::FromString("Dynamic Campaigns"));
	if (CancelButton) {
		CancelButton->OnClicked.AddDynamic(this, &UCampaignScreen::OnCancelButtonClicked);
		CancelButton->OnHovered.AddDynamic(this, &UCampaignScreen::OnCancelButtonHovered);
		CancelButton->OnUnhovered.AddDynamic(this, &UCampaignScreen::OnCancelButtonUnHovered);
		if (CancelButtonText) {
			CancelButtonText->SetText(FText::FromString("CANCEL"));
		}
	}
	if (ApplyButton) {
		ApplyButton->OnClicked.AddDynamic(this, &UCampaignScreen::OnApplyButtonClicked);
		ApplyButton->OnHovered.AddDynamic(this, &UCampaignScreen::OnApplyButtonHovered);
		ApplyButton->OnUnhovered.AddDynamic(this, &UCampaignScreen::OnApplyButtonUnHovered);
		ApplyButton->SetIsEnabled(false);
		if (ApplyButtonText) {
			ApplyButtonText->SetText(FText::FromString("APPLY"));
		}
	}

	CampaignData.SetNum(5);
	ReadCampaignData();
	

}

void UCampaignScreen::OnApplyButtonClicked()
{
}

void UCampaignScreen::OnApplyButtonHovered()
{
}

void UCampaignScreen::OnApplyButtonUnHovered()
{
}

void UCampaignScreen::OnCancelButtonClicked()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->ToggleCampaignScreen(false);
}

void UCampaignScreen::OnCancelButtonHovered()
{
}

void UCampaignScreen::OnCancelButtonUnHovered()
{
}

void UCampaignScreen::ReadCampaignData()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	// Loop through all rows
	
	static const FString ContextString(TEXT("ReadDataTable"));
	TArray<FS_Campaign*> AllRows;
	SSWInstance->CampaignDataTable->GetAllRows<FS_Campaign>(ContextString, AllRows);

	int index = 0;
	for (FS_Campaign* Row : AllRows)
	{
		if (Row)
		{
			UE_LOG(LogTemp, Log, TEXT("Campaign Name: %s"), *Row->Name);
			CampaignData[index] = *Row;
			index++;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to load Campaign!"));
		}
	}

	if (PlayerNameText) {
		PlayerNameText->SetText(FText::FromString(SSWInstance->PlayerInfo.Name));
		UE_LOG(LogTemp, Log, TEXT("Player Name: %s"), *SSWInstance->PlayerInfo.Name);
	}
}

void UCampaignScreen::SetSelectedData(int selected)
{
	CampaignData[selected].Orders.SetNum(4);

	if (NameText) {
		NameText->SetText(FText::FromString(CampaignData[selected].Name));
	}

	if (DescriptionText) {
		DescriptionText->SetText(FText::FromString(CampaignData[selected].Description));
	}

	if (SituationText) {
		SituationText->SetText(FText::FromString(CampaignData[selected].Situation));
	}

	if(Orders1Text) {
		
		Orders1Text->SetText(FText::FromString(CampaignData[selected].Orders[0]));
	}
	if (Orders2Text) {

		Orders2Text->SetText(FText::FromString(CampaignData[selected].Orders[1]));
	}
	if (Orders3Text) {

		Orders3Text->SetText(FText::FromString(CampaignData[selected].Orders[2]));
	}
	if (Orders4Text) {

		Orders4Text->SetText(FText::FromString(CampaignData[selected].Orders[3]));
	}
	if (LocationSystemText) {

		LocationSystemText->SetText(FText::FromString(CampaignData[selected].System));
	}
	if (LocationRegionText) {

		LocationRegionText->SetText(FText::FromString(CampaignData[selected].Region));
	}
}
