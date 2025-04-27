// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OOBStationWidget.h"
#include "Components/TextBlock.h"

void UOOBStationWidget::NativeConstruct()
{
    Super::NativeConstruct();
    // No special setup needed yet
}

void UOOBStationWidget::SetData(const FS_OOBStation& InStation)
{
    Data = InStation;

    if (Label)
    {
        Label->SetText(FText::FromString(Data.Name));
    }
}


