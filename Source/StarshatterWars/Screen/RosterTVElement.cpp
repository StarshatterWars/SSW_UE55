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
FString
URosterTVElement::GetOrdinal(int id)
{
	FString ordinal;

	int last_two_digits = id % 100;

	if (last_two_digits > 10 && last_two_digits < 20) {
		ordinal = FString::FormatAsNumber(id) + "th";
	}
	else {
		int last_digit = last_two_digits % 10;

		if (last_digit == 1)
			ordinal = FString::FormatAsNumber(id) + "st";
		else if (last_digit == 2)
			ordinal = FString::FormatAsNumber(id) + "nd";
		else if (last_digit == 3)
			ordinal = FString::FormatAsNumber(id) + "rd";
		else
			ordinal = FString::FormatAsNumber(id) + "th";
	}

	return ordinal;
}

void URosterTVElement::Setup(UCombatGroupObject* InGroup)
{
	if (!InGroup) return;

	// Example: Add left margin spacing based on indent level
	const float IndentPerLevel = 20.0f;
	IndentSlot->SetPadding(FMargin(IndentPerLevel * InGroup->IndentLevel, 0, 0, 0));

	RosterNameText->SetText(FText::FromString(InGroup->GroupData.Name));
}


