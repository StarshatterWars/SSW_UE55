// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OOBDesronWidget.h"
#include "Components/TextBlock.h"

void UOOBDesronWidget::NativeConstruct()
{
    Super::NativeConstruct();
    // No special setup needed yet
}

void UOOBDesronWidget::SetData(const FS_OOBDestroyer& InDesron)
{
    Data = InDesron;

    if (Label)
    {
        Label->SetText(FText::FromString(Data.Name));
    }
}

















