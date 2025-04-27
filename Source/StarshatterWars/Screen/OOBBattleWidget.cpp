// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OOBBattleWidget.h"
#include "Components/TextBlock.h"

void UOOBBattleWidget::NativeConstruct()
{
    Super::NativeConstruct();
    // No special logic needed here yet
}

void UOOBBattleWidget::SetData(const FS_OOBBattle& InBattle)
{
    Data = InBattle;

    if (Label)
    {
        Label->SetText(FText::FromString(Data.Name));
    }
}

