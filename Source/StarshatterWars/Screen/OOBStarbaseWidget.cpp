// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OOBStarbaseWidget.h"
#include "Components/TextBlock.h"

void UOOBStarbaseWidget::NativeConstruct()
{
    Super::NativeConstruct();
    // No special setup needed yet
}

void UOOBStarbaseWidget::SetData(const FS_OOBStarbase& InStarbase)
{
    Data = InStarbase;

    if (Label)
    {
        Label->SetText(FText::FromString(Data.Name));
    }
}

