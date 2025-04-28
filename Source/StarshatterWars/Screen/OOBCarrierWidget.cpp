// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OOBCarrierWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "OOBCarrierGroupItem.h"
#include "OOBWingItem.h"
#include "OOBUnitItem.h"
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

    if (ElementListView) ElementListView->SetVisibility(bIsExpanded ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    if (WingListView) WingListView->SetVisibility(bIsExpanded ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
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

        if (ElementListView) ElementListView->ClearListItems();
        if (WingListView) WingListView->ClearListItems();

        BuildChildren(CarrierData->Data);
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

    if (ElementListView) ElementListView->SetVisibility(bIsExpanded ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    if (WingListView) WingListView->SetVisibility(bIsExpanded ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UOOBCarrierWidget::BuildChildren(const FS_OOBCarrier& CarrierDataStruct)
{
    if (!ElementListView || !WingListView) {
        UE_LOG(LogTemp, Error, TEXT("Carrier ListViews are not valid!"));
        return;
    }

    WingListView->ClearListItems();

    // Fill Wings
    for (const FS_OOBWing& Wing : CarrierDataStruct.Wing)
    {
        UE_LOG(LogTemp, Error, TEXT("Wing Found: %s"), *Wing.Name);
        UOOBWingItem* WingData = NewObject<UOOBWingItem>(this);
        if (WingData)
        {
            WingData->Data = Wing;
            WingListView->AddItem(WingData);
        }
    }

    // Fill Ship Elements
    for (const FS_OOBUnit& Ship : CarrierDataStruct.Unit)
    {
        UE_LOG(LogTemp, Error, TEXT("Ship Found: %s"), *Ship.Name);
        UOOBUnitItem* UnitData = NewObject<UOOBUnitItem>(this);
        
        if (UnitData)
        {
            UnitData->Data = Ship;
            ElementListView->AddItem(UnitData);
        }
    }
}

