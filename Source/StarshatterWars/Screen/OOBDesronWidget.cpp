// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OOBDesronWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/HorizontalBoxSlot.h" 

void UOOBDesronWidget::NativeConstruct()
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

    if (ExpandIcon)
    {
        ExpandIcon->SetVisibility(ESlateVisibility::Visible);

        if (bIsExpanded)
        {
            ExpandIcon->SetBrushFromTexture(ExpandedIconTexture); // Expanded
        }
        else
        {
            ExpandIcon->SetBrushFromTexture(CollapsedIconTexture); // Collapsed
        }
    }
}

void UOOBDesronWidget::SetData(const FS_OOBDestroyer& InDesron, int32 InIndentLevel)
{
    IndentLevel = InIndentLevel;
    Data = InDesron;

    if (NameText)
    {
        NameText->SetText(FText::FromString(Data.Name));
    }
}

void UOOBDesronWidget::BuildChildren()
{
    Children.Empty();
}
















