// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OOBStationWidget.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBoxSlot.h" 

void UOOBStationWidget::NativeConstruct()
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

void UOOBStationWidget::SetData(const FS_OOBStation& InStation, int32 InIndentLevel)
{
    IndentLevel = InIndentLevel; 
    Data = InStation;

    if (NameText)
    {
        NameText->SetText(FText::FromString(Data.Name));
    }
}


