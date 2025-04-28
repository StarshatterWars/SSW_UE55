// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OOBCarrierWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "OOBCarrierGroupItem.h"
#include "Components/ListView.h" 

void UOOBCarrierWidget::NativeConstruct()
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

    if (WingListView)
    {
        WingListView->ClearListItems(); // Empty on construct
        WingListView->SetVisibility(ESlateVisibility::Collapsed); // Hide initially
    }
}

void UOOBCarrierWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    if (UOOBCarrierGroupItem* CarrierData = Cast<UOOBCarrierGroupItem>(ListItemObject))
    {
        if (NameText)
        {
            NameText->SetText(FText::FromString(CarrierData->Data.Name));
        }

        // Expand/collapse setup
        bIsExpanded = false;

        // Build children Fleets based on full struct
        if (WingListView)
        {
            WingListView->ClearListItems();
            BuildChildren(CarrierData->Data);
        }
    }
}


FReply UOOBCarrierWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    ToggleExpansion(); // << Expand or collapse when clicked
    return FReply::Handled();
}

void UOOBCarrierWidget::ToggleExpansion()
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

    if (WingListView)
    {
        WingListView->SetVisibility(bIsExpanded ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}

void UOOBCarrierWidget::BuildChildren(const FS_OOBCarrier& CarrierDataStruct)
{
    if (!WingListView) return;

    WingListView->ClearListItems();
}

