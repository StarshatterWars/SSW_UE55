// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OOBForceWidget.h"
#include "OOBForceItem.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "../Game/GameStructs.h" // FS_OOBFleet definition
#include "OOBFleetWidget.h"
#include "OOBBattalionWidget.h"
#include "Components/ListView.h"
#include "Components/HorizontalBoxSlot.h" 

void UOOBForceWidget::NativeConstruct()
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

    if (FleetListView)
    {
        FleetListView->ClearListItems(); // Empty list on construct
    }
}

void UOOBForceWidget::SetForceData(const FS_OOBForce& InForce, int32 InIndentLevel)
{
    ForceData = InForce;
    IndentLevel = InIndentLevel;

    if (NameText)
    {
        NameText->SetText(FText::FromString(ForceData.Name));
    }

    if (FleetListView)
    {
        FleetListView->ClearListItems();
    }

    BuildChildren();
}

void UOOBForceWidget::BuildChildren()
{
    Children.Empty();

    // Add each Fleet inside the Force to the ListView
    for (const FS_OOBFleet& Fleet : ForceData.Fleet)
    {
        UOOBFleetWidget* FleetItem = CreateWidget<UOOBFleetWidget>(GetWorld(), UOOBFleetWidget::StaticClass());
        if (FleetItem)
        {
            FleetItem->SetFleetData(Fleet, IndentLevel + 1); // 1 level deeper for Fleets
            FleetListView->AddItem(FleetItem);
            Children.Add(FleetItem);
        }
    }
}

void UOOBForceWidget::ToggleExpansion()
{
    bIsExpanded = !bIsExpanded;

    if (ExpandIcon)
    {
        if (bIsExpanded)
        {
            ExpandIcon->SetBrushFromTexture(ExpandedIconTexture);
        }
        else
        {
            ExpandIcon->SetBrushFromTexture(CollapsedIconTexture);
        }
    }

    if (FleetListView)
    {
        FleetListView->SetVisibility(bIsExpanded ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}