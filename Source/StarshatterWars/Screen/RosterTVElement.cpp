// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "RosterTVElement.h"
#include "OperationsScreen.h"
#include "../System/SSWGameInstance.h"

void URosterTVElement::NativeConstruct()
{
	Super::NativeConstruct();

	if (RosterButton) {
		RosterButton->OnClicked.AddDynamic(this, &URosterTVElement::OnRosterButtonClicked);
	}
}

void URosterTVElement::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance(); 
	RosterView = Cast<URosterViewObject>(ListItemObject);

	if (!RosterView) {
		return;
	}

	if (UListView* OwningListView = Cast<UListView>(GetOwningListView())) {
		RosterId = OwningListView->GetIndexForItem(RosterView);
	}
	if (RosterNameText)
	{
		RosterNameText->SetText(FText::FromString(RosterView->Group.DisplayName));
	}
	
	if (RosterLocationText)
	{
		RosterLocationText->SetText(FText::FromString(RosterView->Group.Region));
	}
}

void URosterTVElement::OnRosterButtonClicked()
{
	UE_LOG(LogTemp, Log, TEXT("Selected Roster Is: %i"), RosterId);
	SetRosterInfo();
}

void URosterTVElement::SetRosterInfo()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->SetSelectedRosterNr(RosterId);
	SSWInstance->RosterSelectionChanged = true;
}




