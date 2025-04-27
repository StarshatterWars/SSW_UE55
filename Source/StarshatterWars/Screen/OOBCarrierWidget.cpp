// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OOBCarrierWidget.h"
#include "Components/TextBlock.h"

void UOOBCarrierWidget::NativeConstruct()
{
    Super::NativeConstruct();
    // Nothing extra needed yet
}

void UOOBCarrierWidget::SetData(const FS_OOBCarrier& InCarrier)
{
    Data = InCarrier;

    if (Label)
    {
        Label->SetText(FText::FromString(Data.Name));
    }
}


