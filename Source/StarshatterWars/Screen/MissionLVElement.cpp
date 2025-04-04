// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "MissionLVElement.h"
#include "OperationsScreen.h"
#include "../System/SSWGameInstance.h"

void UMissionLVElement::NativeConstruct()
{
	if (MissionButton) {
		MissionButton->OnClicked.AddDynamic(this, &UMissionLVElement::OnMissionButtonClicked);
	}
}

void UMissionLVElement::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	MissionList = Cast<UMissionListObject>(ListItemObject);
	
	if(!MissionList) {
		return;
	}
	
	if (UListView* OwningListView = Cast<UListView>(GetOwningListView())) {
		MissionId = OwningListView->GetIndexForItem(MissionList);
	}

	if (MissionName)
	{
		MissionName->SetText(FText::FromString(MissionList->MissionName));
	}
	if (MissionStatus)
	{
		MissionStatus->SetText(FText::FromString(MissionList->MissionStatus));
	}
	if (MissionTime)
	{
		MissionTime->SetText(FText::FromString(MissionList->MissionTime));
	}
	if (MissionType)
	{
		MissionType->SetText(FText::FromString(MissionList->MissionType));
	}
}

void UMissionLVElement::SetMissionStatus()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->SetSelectedMissionNr(MissionId);
	SSWInstance->MissionSelectionChanged = true;
}

void UMissionLVElement::OnMissionButtonClicked()
{
	UE_LOG(LogTemp, Log, TEXT("Selected Mission Nr: %i"), MissionId);
	SetMissionStatus();
}

