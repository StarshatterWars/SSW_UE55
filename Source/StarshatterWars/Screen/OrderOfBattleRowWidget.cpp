// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OrderOfBattleRowWidget.h"
#include "OrderOfBattleRowObject.h"
#include "Components/TextBlock.h"

void UOrderOfBattleRowWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	if (UOrderOfBattleRowObject* Data = Cast<UOrderOfBattleRowObject>(ListItemObject))
	{
		if (NameText)
		{
			NameText->SetText(FText::FromString(Data->Name));
		}
	}
}








