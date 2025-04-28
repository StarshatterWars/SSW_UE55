// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OOBForceWidget.h"
#include "OOBFleetItem.h"
#include "OOBForceItem.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "../Game/GameStructs.h" // FS_OOBFleet definition
#include "Components/ListView.h" 

void UOOBForceWidget::NativeConstruct()
{
    Super::NativeConstruct();

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
        FleetListView->ClearListItems(); // Empty on construct
        FleetListView->SetVisibility(ESlateVisibility::Collapsed); // Hide initially
    }
 }

void UOOBForceWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    if (UOOBForceItem* ForceData = Cast<UOOBForceItem>(ListItemObject))
    {
        if (NameText)
        {
            NameText->SetText(FText::FromString(ForceData->Data.Name));
        }

        // Expand/collapse setup
        bIsExpanded = false;

        // Build children Fleets based on full struct
        if (FleetListView)
        {
            FleetListView->ClearListItems();
            BuildChildren(ForceData->Data);
        }
    }
}


FReply UOOBForceWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    ToggleExpansion(); // << Expand or collapse when clicked
    return FReply::Handled();
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

void UOOBForceWidget::BuildChildren(const FS_OOBForce& ForceDataStruct)
{
    if (!FleetListView) return;

    FleetListView->ClearListItems();

    for (const FS_OOBFleet& Fleet : ForceDataStruct.Fleet)
    {
        UOOBFleetItem* FleetData = NewObject<UOOBFleetItem>(this);
        if (FleetData)
        {
            FleetData->Data = Fleet;

            FleetListView->AddItem(FleetData);
        }
    }
}
