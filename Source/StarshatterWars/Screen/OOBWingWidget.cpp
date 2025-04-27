// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OOBWingWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/HorizontalBoxSlot.h"

void UOOBWingWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (UHorizontalBoxSlot* HBoxSlot = Cast<UHorizontalBoxSlot>(Slot))
    {
        const float IndentSize = 20.0f;
        HBoxSlot->SetPadding(FMargin(IndentLevel * IndentSize, 0.0f, 0.0f, 0.0f));
    }

    if (ExpandIcon)
    {
        ExpandIcon->SetVisibility(ESlateVisibility::Visible);

        if (bIsExpanded)
        {
            ExpandIcon->SetBrushFromTexture(ExpandedIconTexture); // Expanded (-)
        }
        else
        {
            ExpandIcon->SetBrushFromTexture(CollapsedIconTexture); // Collapsed (+)
        }
    }
}

void UOOBWingWidget::SetData(const FS_OOBWing& InWing, int32 InIndentLevel)
{
    Data = InWing;
    IndentLevel = InIndentLevel;

    if (NameText)
    {
        NameText->SetText(FText::FromString(Data.Name));
    }

    Children.Empty();
}

void UOOBWingWidget::BuildChildren()
{
    Children.Empty();
}



