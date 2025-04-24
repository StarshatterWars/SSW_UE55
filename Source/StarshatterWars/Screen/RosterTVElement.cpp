// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "RosterTVElement.h"

#include "OOBForceItem.h"
#include "OOBFleetItem.h"
#include "OOBCarrierGroupItem.h"
#include "OOBBattleItem.h"
#include "OOBDestroyerItem.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

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

    /*if (!IsValid(NameText))
    {
        return;
    }

    // Check type and set display text accordingly
    if (const UOOBForceItem* ForceItem = Cast<UOOBForceItem>(ListItemObject))
    {
        NameText->SetText(FText::FromString(FString::Printf(TEXT("[FORCE] %s"), *ForceItem->Data.Name)));
    }
    else if (const UOOBFleetItem* FleetItem = Cast<UOOBFleetItem>(ListItemObject))
    {
        NameText->SetText(FText::FromString(FString::Printf(TEXT("[FLEET] %s"), *FleetItem->Data.Name)));
    }
    else if (const UOOBCarrierGroupItem* CarrierItem = Cast<UOOBCarrierGroupItem>(ListItemObject))
    {
        NameText->SetText(FText::FromString(FString::Printf(TEXT("[CARRIER] %s"), *CarrierItem->Data.Name)));
    }
    else if (const UOOBBattleItem* BattleItem = Cast<UOOBBattleItem>(ListItemObject))
    {
        NameText->SetText(FText::FromString(FString::Printf(TEXT("[BATTLE] %s"), *BattleItem->Data.Name)));
    }
    else if (const UOOBDestroyerItem* DesRonItem = Cast<UOOBDestroyerItem>(ListItemObject))
    {
        NameText->SetText(FText::FromString(FString::Printf(TEXT("[DESRON] %s"), *DesRonItem->Data.Name)));
    }
    else
    {
        // Fallback if the type is not recognized
        NameText->SetText(FText::FromString(TEXT("Unknown Item")));
    }*/
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




