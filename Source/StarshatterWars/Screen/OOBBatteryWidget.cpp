// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OOBBatteryWidget.h"
#include "Components/TextBlock.h"

void UOOBBatteryWidget::NativeConstruct()
{
    Super::NativeConstruct();
    // No extra setup needed yet
}

void UOOBBatteryWidget::SetData(const FS_OOBBattery& InBattery)
{
    Data = InBattery;

    if (Label)
    {
        Label->SetText(FText::FromString(Data.Name));
    }
}


