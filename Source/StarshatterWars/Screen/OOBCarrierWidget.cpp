// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OOBCarrierWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "OOBWingWidget.h"
#include "Components/HorizontalBoxSlot.h" 

void UOOBCarrierWidget::NativeConstruct()
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

void UOOBCarrierWidget::SetData(const FS_OOBCarrier& InCarrier, int32 InIndentLevel)
{
    IndentLevel = InIndentLevel;
    Data = InCarrier;

    if (NameText)
    {
        NameText->SetText(FText::FromString(Data.Name));
    }
}

void UOOBCarrierWidget::BuildChildren()
{
    Children.Empty();

    // Carrier now expands ONLY into Wings
    for (const FS_OOBWing& Wing : Data.Wing)
    {
        UOOBWingWidget* WingItem = CreateWidget<UOOBWingWidget>(GetWorld(), UOOBWingWidget::StaticClass());
        if (WingItem)
        {
            WingItem->SetData(Wing, IndentLevel + 1); // Wings are indented 1 deeper
            Children.Add(WingItem);
        }
    }
}


