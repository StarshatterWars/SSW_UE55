// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "CampaignScreen.h"

void UCampaignScreen::NativeConstruct()
{
	Super::NativeConstruct();

	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();

	if (Title)
		Title->SetText(FText::FromString("Campaign"));
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
