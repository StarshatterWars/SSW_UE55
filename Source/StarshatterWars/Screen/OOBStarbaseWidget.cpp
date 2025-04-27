// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OOBStarbaseWidget.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBoxSlot.h" 

void UOOBStarbaseWidget::NativeConstruct()
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

void UOOBStarbaseWidget::SetData(const FS_OOBStarbase& InStarbase, int32 InIndentLevel)
{
    IndentLevel = InIndentLevel;
    Data = InStarbase;

    if (NameText)
    {
        NameText->SetText(FText::FromString(Data.Name));
    }
}

