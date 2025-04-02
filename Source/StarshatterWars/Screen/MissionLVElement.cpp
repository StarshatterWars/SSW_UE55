// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "MissionLVElement.h"
#include "../System/SSWGameInstance.h"

void UMissionLVElement::NativeConstruct()
{
	if (MissionButton) {
		MissionButton->OnClicked.AddDynamic(this, &UMissionLVElement::OnMissionButtonClicked);
	}
}

/*void UMissionLVElement::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	if (UMissionListObject* Data = Cast<UMissionListObject>(ListItemObject))
	{
		//if (MissionId)
		//{
		//	MissionId = Data->MissionId;
		//}
		if (MissionName)
		{
			MissionName->SetText(FText::FromString(Data->MissionName));
		}
		//if (MissionStatus)
		//{
		//	MissionStatus->SetText(FText::FromString(Data->MissionStatus));
		//}
		//if (MissionTime)
		//{
		//	MissionTime->SetText(FText::FromString(Data->MissionTime));
		//}
		//if (MissionType)
		//{
		//	MissionType->SetText(FText::FromString(Data->MissionType));
		//}
		UE_LOG(LogTemp, Log, TEXT("LVE Mission Name: %s"), *Data->MissionName);
	}
}*/

void UMissionLVElement::SetMissionStatus()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->SelectionMissionNr = MissionId;
	SSWInstance->MissionSelectionChanged = true;
}

void UMissionLVElement::OnMissionButtonClicked()
{
	SetMissionStatus();
}

