// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OOBFleetWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "OOBFleetItem.h"
#include "OOBBattleItem.h"
#include "OOBDestroyerItem.h"
#include "OOBCarrierGroupItem.h"
#include "Components/ListView.h"

void UOOBFleetWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (ExpandIcon)
    {
        ExpandIcon->SetVisibility(ESlateVisibility::Visible);
        ExpandIcon->SetBrushFromTexture(CollapsedIconTexture);
    }

    if (BattleListView) BattleListView->SetVisibility(bIsExpanded ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    if (CarrierListView) CarrierListView->SetVisibility(bIsExpanded ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    if (DestroyerListView) DestroyerListView->SetVisibility(bIsExpanded ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UOOBFleetWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    if (UOOBFleetItem* FleetData = Cast<UOOBFleetItem>(ListItemObject))
    {
        if (NameText)
        {
           NameText->SetText(FText::FromString(FleetData->Data.Name));
        }

        bIsExpanded = false;

        if (BattleListView) BattleListView->ClearListItems();
        if (CarrierListView) CarrierListView->ClearListItems();
        if (DestroyerListView) DestroyerListView->ClearListItems();

        BuildChildren(FleetData->Data);
    }
}

FReply UOOBFleetWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    ToggleExpansion();
    return FReply::Handled();
}

void UOOBFleetWidget::ToggleExpansion()
{
    bIsExpanded = !bIsExpanded;

    if (ExpandIcon)
    {
        ExpandIcon->SetBrushFromTexture(bIsExpanded ? ExpandedIconTexture : CollapsedIconTexture);
    }

    if (BattleListView) BattleListView->SetVisibility(bIsExpanded ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    if (CarrierListView) CarrierListView->SetVisibility(bIsExpanded ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    if (DestroyerListView) DestroyerListView->SetVisibility(bIsExpanded ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UOOBFleetWidget::BuildChildren(const FS_OOBFleet& FleetDataStruct)
{
    if (!BattleListView || !CarrierListView || !DestroyerListView) return;

    // Fill Battles
    for (const FS_OOBBattle& Battle : FleetDataStruct.Battle)
    {
        // Create BattleDataObject
        // BattlesListView->AddItem(BattleDataObject);
    }

    // Fill Carriers
    for (const FS_OOBCarrier& Carrier : FleetDataStruct.Carrier)
    {
        // Create CarrierDataObject
        // CarriersListView->AddItem(CarrierDataObject);
    }

    // Fill DESRONs
    for (const FS_OOBDestroyer& Desron : FleetDataStruct.Destroyer)
    {
        // Create DesronDataObject
        // DesronsListView->AddItem(DesronDataObject);
    }
}
