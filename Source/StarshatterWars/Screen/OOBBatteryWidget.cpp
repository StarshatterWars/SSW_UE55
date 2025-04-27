// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OOBBatteryWidget.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBoxSlot.h" 

void UOOBBatteryWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    if (NameText)
    {
        if (UHorizontalBoxSlot* HBoxSlot = Cast<UHorizontalBoxSlot>(NameText->Slot))
        {
            const float IndentSize = 20.0f;
            HBoxSlot->SetPadding(FMargin(IndentLevel * IndentSize, 0.0f, 0.0f, 0.0f));
        }
    }
}

void UOOBBatteryWidget::SetData(const FS_OOBBattery& InBattery, int32 InIndentLevel)
{
    IndentLevel = InIndentLevel;
    Data = InBattery;

    if (NameText)
    {
        NameText->SetText(FText::FromString(Data.Name));
    }
}


