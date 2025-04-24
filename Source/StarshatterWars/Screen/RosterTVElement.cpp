// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "RosterTVElement.h"

#include "OOBForceItem.h"
#include "OOBFleetItem.h"
#include "OOBCarrierGroupItem.h"
#include "OOBBattleItem.h"
#include "OOBDestroyerItem.h"
#include "OOBWingItem.h"
#include "OOBUnitItem.h"
#include "OOBSquadronItem.h"
#include "OOBFighterSquadronItem.h"

#include "Components/TextBlock.h"
#include "Components/Button.h"

#include "OperationsScreen.h"
#include "../System/SSWGameInstance.h"

void URosterTVElement::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance(); 

	if (!NameText || !ListItemObject) return;

    if (!IsValid(NameText))
    {
        return;
    }

    // Check type and set display text accordingly
    if (const UOOBForceItem* ForceItem = Cast<UOOBForceItem>(ListItemObject))
    {
        NameText->SetText(FText::FromString(FString::Printf(TEXT("%s"), *ForceItem->Data.Name)));
    }
    else if (const UOOBFleetItem* FleetItem = Cast<UOOBFleetItem>(ListItemObject))
    {
        NameText->SetText(FText::FromString(FString::Printf(TEXT("%s"), *FleetItem->Data.Name)));
    }
    else if (const UOOBCarrierGroupItem* CarrierItem = Cast<UOOBCarrierGroupItem>(ListItemObject))
    {
        NameText->SetText(FText::FromString(FString::Printf(TEXT("%s"), *CarrierItem->Data.Name)));
    }
    else if (const UOOBBattleItem* BattleItem = Cast<UOOBBattleItem>(ListItemObject))
    {  
        NameText->SetText(FText::FromString(FString::Printf(TEXT("%s"), *BattleItem->Data.Name)));
    }
    else if (const UOOBDestroyerItem* DesronItem = Cast<UOOBDestroyerItem>(ListItemObject))
    {
        NameText->SetText(FText::FromString(FString::Printf(TEXT("%s"), *DesronItem->Data.Name)));
    }
    else if (const UOOBWingItem* WingItem = Cast<UOOBWingItem>(ListItemObject))
    {
        NameText->SetText(FText::FromString(FString::Printf(TEXT("%s"), *WingItem->Data.Name)));
    }
    else if (const UOOBUnitItem* UnitItem = Cast<UOOBUnitItem>(ListItemObject))
    { 
        NameText->SetText(FText::FromString(FString::Printf(TEXT("%s"), *UnitItem->Data.DisplayName)));
    }
    else if (const UOOBFighterSquadronItem* FighterSquadronItem = Cast<UOOBFighterSquadronItem>(ListItemObject))
    {
        NameText->SetText(FText::FromString(FString::Printf(TEXT("%s"), *FighterSquadronItem->Data.Name)));
    }
    else if (const UOOBSquadronItem* SquadronItem = Cast<UOOBSquadronItem>(ListItemObject))
    {
        NameText->SetText(FText::FromString(FString::Printf(TEXT("%s"), *SquadronItem->GetDisplayName())));
    }
    else
    {
        // Fallback if the type is not recognized
        NameText->SetText(FText::FromString(TEXT("Unknown Item")));
    }
}





