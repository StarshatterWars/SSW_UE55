// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OrderOfBattleListEntry.h"
#include "OrderOfBattleWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "OrderOfBattleRowObject.h"

void UOrderOfBattleListEntry::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UOrderOfBattleListEntry::Setup(UOrderOfBattleRowObject* Data)
{
	if (Data)
	{
		RowData = Data;
		DisplayNameText->SetText(FText::FromString(Data->DisplayName));
	}
}