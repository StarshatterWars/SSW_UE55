// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OperationsScreen.h"
#include "MissionListObject.h"

void UOperationsScreen::NativeConstruct()
{
	Super::NativeConstruct();

	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->LoadGame(SSWInstance->PlayerSaveName, SSWInstance->PlayerSaveSlot);

	if (TitleText)
		TitleText->SetText(FText::FromString("Operational Command").ToUpper());
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

	if (OrdersButton) {
		OrdersButton->OnClicked.AddDynamic(this, &UOperationsScreen::OnOrdersButtonClicked);
		OrdersButton->OnHovered.AddDynamic(this, &UOperationsScreen::OnOrdersButtonHovered);
		OrdersButton->OnUnhovered.AddDynamic(this, &UOperationsScreen::OnOrdersButtonUnHovered);
	}

	if (TheaterButton) {
		TheaterButton->OnClicked.AddDynamic(this, &UOperationsScreen::OnTheaterButtonClicked);
		TheaterButton->OnHovered.AddDynamic(this, &UOperationsScreen::OnTheaterButtonHovered);
		TheaterButton->OnUnhovered.AddDynamic(this, &UOperationsScreen::OnTheaterButtonUnHovered);
	}

	if (ForcesButton) {
		ForcesButton->OnClicked.AddDynamic(this, &UOperationsScreen::OnForcesButtonClicked);
		ForcesButton->OnHovered.AddDynamic(this, &UOperationsScreen::OnForcesButtonHovered);
		ForcesButton->OnUnhovered.AddDynamic(this, &UOperationsScreen::OnForcesButtonUnHovered);
	}

	if (IntelButton) {
		IntelButton->OnClicked.AddDynamic(this, &UOperationsScreen::OnIntelButtonClicked);
		IntelButton->OnHovered.AddDynamic(this, &UOperationsScreen::OnIntelButtonHovered);
		IntelButton->OnUnhovered.AddDynamic(this, &UOperationsScreen::OnIntelButtonUnHovered);
	}

	if (MissionsButton) {
		MissionsButton->OnClicked.AddDynamic(this, &UOperationsScreen::OnMissionsButtonClicked);
		MissionsButton->OnHovered.AddDynamic(this, &UOperationsScreen::OnMissionsButtonHovered);
		MissionsButton->OnUnhovered.AddDynamic(this, &UOperationsScreen::OnMissionsButtonUnHovered);
	}
	if (PlayerNameText) {
		PlayerNameText->SetText(FText::FromString(SSWInstance->PlayerInfo.Name));
	}

	if (OperationalSwitcher) {
		OperationalSwitcher->SetActiveWidgetIndex(0);
	}

	if (OperationsModeText) {
		OperationsModeText->SetText(FText::FromString("ORDERS"));
	}

	ActiveCampaign = SSWInstance->GetActiveCampaign();
	
	if (SSWInstance->GetActiveCampaign().Index == 0) {
		if (TheaterButton) {
			TheaterButton->SetIsEnabled(false);
		}
		if (ForcesButton) {
			ForcesButton->SetIsEnabled(false); 
		}
		if (IntelButton) {
			IntelButton->SetIsEnabled(false);
		}
	} else {
		if (TheaterButton) {
				TheaterButton->SetIsEnabled(true);
		}
		if (ForcesButton) {
			ForcesButton->SetIsEnabled(true);
		}
		if (IntelButton) {
			IntelButton->SetIsEnabled(true);
		}
	}


	SetCampaignOrders();
	PopulateMissionList();
	SetCampaignMissions();
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

void UOperationsScreen::OnOrdersButtonClicked()
{
	if (OperationalSwitcher) {
		OperationalSwitcher->SetActiveWidgetIndex(0);
	}
	if (OperationsModeText) {
		OperationsModeText->SetText(FText::FromString("ORDERS"));
	}
	SetCampaignOrders();
}

void UOperationsScreen::OnOrdersButtonHovered()
{
}

void UOperationsScreen::OnOrdersButtonUnHovered()
{
}

void UOperationsScreen::OnTheaterButtonClicked()
{
	if (OperationalSwitcher) {
		OperationalSwitcher->SetActiveWidgetIndex(1);
	}

	if (OperationsModeText) {
		OperationsModeText->SetText(FText::FromString("THEATER"));
	}
}

void UOperationsScreen::OnTheaterButtonHovered()
{
}

void UOperationsScreen::OnTheaterButtonUnHovered()
{
}

void UOperationsScreen::OnForcesButtonClicked()
{
	if (OperationalSwitcher) {
		OperationalSwitcher->SetActiveWidgetIndex(2);
	}
	if (OperationsModeText) {
		OperationsModeText->SetText(FText::FromString("FORCES"));
	}
}

void UOperationsScreen::OnForcesButtonHovered()
{
}

void UOperationsScreen::OnForcesButtonUnHovered()
{
}

void UOperationsScreen::OnIntelButtonClicked()
{
	if (OperationalSwitcher) {
		OperationalSwitcher->SetActiveWidgetIndex(3);
	}

	if (OperationsModeText) {
		OperationsModeText->SetText(FText::FromString("INTEL"));
	}
}

void UOperationsScreen::OnIntelButtonHovered()
{
}

void UOperationsScreen::OnIntelButtonUnHovered()
{
}

void UOperationsScreen::OnMissionsButtonClicked()
{
	if (OperationalSwitcher) {
		OperationalSwitcher->SetActiveWidgetIndex(4);
	}

	if (OperationsModeText) {
		OperationsModeText->SetText(FText::FromString("MISSIONS"));
	}
	SetCampaignMissions();
	PopulateMissionList(); 	
}

void UOperationsScreen::OnMissionsButtonHovered()
{
}

void UOperationsScreen::OnMissionsButtonUnHovered()
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

void UOperationsScreen::SetCampaignOrders()
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

void UOperationsScreen::SetCampaignMissions()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	
	TArray<FS_CampaignMissionList> Missions = SSWInstance->GetActiveCampaign().MissionList;

	for (FS_CampaignMissionList info : Missions)
	{
		FString NewMission = info.Name;
		UE_LOG(LogTemp, Log, TEXT("Mission Name Selected: %s"), *NewMission);
	}
}

void UOperationsScreen::PopulateMissionList()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance(); 
	
	if (!MissionList) return;

	MissionList->ClearListItems();

	for (int32 i = 0; i < SSWInstance->GetActiveCampaign().MissionList.Num(); ++i)
	{
		UMissionListObject* ListItem = NewObject<UMissionListObject>();
		ListItem->MissionName = SSWInstance->GetActiveCampaign().MissionList[i].Name;
		ListItem->MissionType = SSWInstance->GetActiveCampaign().MissionList[i].TypeName;
		ListItem->MissionTime = SSWInstance->GetActiveCampaign().MissionList[i].Start;
		ListItem->MissionRegion = SSWInstance->GetActiveCampaign().MissionList[i].Region;
		ListItem->MissionSystem = SSWInstance->GetActiveCampaign().MissionList[i].System;
		ListItem->MissionSitrep = SSWInstance->GetActiveCampaign().MissionList[i].System;
		
		if (SSWInstance->GetActiveCampaign().MissionList[i].Status == EMISSIONSTATUS::Active) {
			ListItem->MissionStatus = "Active";
		}
		else if (SSWInstance->GetActiveCampaign().MissionList[i].Status == EMISSIONSTATUS::Complete) {
			ListItem->MissionStatus = "Complete";
		}
		else if (SSWInstance->GetActiveCampaign().MissionList[i].Status == EMISSIONSTATUS::Ready) {
			ListItem->MissionStatus = "Ready";
		}
		else if (SSWInstance->GetActiveCampaign().MissionList[i].Status == EMISSIONSTATUS::Available) {
			ListItem->MissionStatus = "Available";
		}
		else if (SSWInstance->GetActiveCampaign().MissionList[i].Status == EMISSIONSTATUS::Pending) {
			ListItem->MissionStatus = "Pending";
		}
		MissionList->AddItem(ListItem);

		UE_LOG(LogTemp, Log, TEXT("Ops Mission Name: %s"), *ListItem->MissionName);
	}
}

