// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OrderOfBattleListEntry.h"
#include "OrderOfBattleWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "CombatGroupObject.h"
#include "OrderOfBattleRowObject.h"

void UOrderOfBattleListEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	if (UCombatGroupObject* GroupObject = Cast<UCombatGroupObject>(ListItemObject))
	{
		if (NameText)
		{
			NameText->SetText(FText::FromString(GroupObject->Data.Name));
		}
		if (TypeText)
		{
			TypeText->SetText(FText::FromString(GroupObject->Data.Type));
		}
	}
}