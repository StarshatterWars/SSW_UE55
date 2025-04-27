// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OOBBattleWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/HorizontalBoxSlot.h" 

void UOOBBattleWidget::NativeConstruct()
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

void UOOBBattleWidget::SetData(const FS_OOBBattle& InBattle, int32 InIndentLevel)
{
    IndentLevel = InIndentLevel;
    Data = InBattle;

    if (NameText)
    {
        NameText->SetText(FText::FromString(Data.Name));
    }
}

void UOOBBattleWidget::BuildChildren()
{
    Children.Empty();
}

