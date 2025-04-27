// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OOBForceWidget.h"
#include "OOBForceItem.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "../Game/GameStructs.h" // FS_OOBFleet definition
#include "OOBFleetWidget.h"
#include "OOBBattalionWidget.h"
#include "Components/HorizontalBoxSlot.h" 

void UOOBForceWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Indent the whole row based on IndentLevel
    if (UHorizontalBoxSlot* HBoxSlot = Cast<UHorizontalBoxSlot>(Slot))
    {
        const float IndentSize = 20.0f;
        HBoxSlot->SetPadding(FMargin(IndentLevel * IndentSize, 0.0f, 0.0f, 0.0f));
    }

    // Update Expand/Collapse Icon (only for Force/Fleet/Battalion types)
    if (ExpandIcon)
    {
        ExpandIcon->SetVisibility(ESlateVisibility::Visible);

        if (bIsExpanded)
        {
            ExpandIcon->SetBrushFromTexture(ExpandedIconTexture); // Minus icon (expanded)
        }
        else
        {
            ExpandIcon->SetBrushFromTexture(CollapsedIconTexture); // Plus icon (collapsed)
        }
    }
}

void UOOBForceWidget::SetData(UOOBForceItem* InForceObject)
{
    Object = InForceObject;

    if (Object && NameText)
    {
        NameText->SetText(FText::FromString(Object->Data.Name));
    }

    Children.Empty(); // Reset children
}

void UOOBForceWidget::BuildChildren()
{
    Children.Empty();

    if (!Object) return;

    // Build Fleets
    for (const FS_OOBFleet& Fleet : Object->Data.Fleet)
    {
        UOOBFleetWidget* FleetItem = CreateWidget<UOOBFleetWidget>(GetWorld(), UOOBFleetWidget::StaticClass());
        if (FleetItem)
        {
            FleetItem->SetData(Fleet, 1);
            Children.Add(FleetItem);
        }
    }

    // Build Battalions
    for (const FS_OOBBattalion& Battalion : Object->Data.Battalion)
    {
        UOOBBattalionWidget* BattalionItem = CreateWidget<UOOBBattalionWidget>(GetWorld(), UOOBBattalionWidget::StaticClass());
        if (BattalionItem)
        {
            BattalionItem->SetData(Battalion);
            Children.Add(BattalionItem);
        }
    }
}

