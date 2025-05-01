// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OOBForceWidget.h"
#include "OOBFleetItem.h"
#include "OOBForceItem.h"
#include "OOBBattalion.h"
#include "OperationsScreen.h"
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

    SetVisible(bIsExpanded);
 }

void UOOBForceWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    if (UOOBForceItem* ForceData = Cast<UOOBForceItem>(ListItemObject))
    {
        if (NameText)
        {
            NameText->SetText(FText::FromString(ForceData->Data.Name));
        }

        Data = ForceData->Data;
        // Expand/collapse setup
        bIsExpanded = false;

        // Build children Fleets based on full struct
        if (FleetListView) { FleetListView->ClearListItems(); }
        if (BattalionListView) { BattalionListView->ClearListItems(); }

        BuildChildren(ForceData->Data);
    }
}

FReply UOOBForceWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    ToggleExpansion(); // << Expand or collapse when clicked
   
    //if (OnForceClicked.IsBound())
    //{
        OnForceClicked.Broadcast(this); // Send this ForceWidget to the screen
    //}
    return FReply::Handled();
}

void UOOBForceWidget::ToggleExpansion()
{
    USSWGameInstance* SSWInstance = Cast<USSWGameInstance>(GetGameInstance());
    SSWInstance->SetActiveUnit(true, Data.Name, Data.Empire, Data.Type, Data.Location);
    SSWInstance->bIsDisplayUnitChanged = true;
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
    
    SetVisible(bIsExpanded);
}

void UOOBForceWidget::SetVisible(bool bIsVisible)
{
    if (FleetListView) FleetListView->SetVisibility(bIsVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    if (BattalionListView) BattalionListView->SetVisibility(bIsVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UOOBForceWidget::BuildChildren(const FS_OOBForce& ForceDataStruct)
{
    if (!FleetListView || !BattalionListView) {
        UE_LOG(LogTemp, Error, TEXT("Force ListViews are not valid!"));
        return;
    }

    for (const FS_OOBFleet& Fleet : ForceDataStruct.Fleet)
    {
        UOOBFleetItem* FleetData = NewObject<UOOBFleetItem>(this);
        if (FleetData)
        {
            FleetData->Data = Fleet;
            FleetListView->AddItem(FleetData);
        }
    }

    for (const FS_OOBBattalion& Battalion : ForceDataStruct.Battalion)
    {
        UOOBBattalion* BattalionData = NewObject<UOOBBattalion>(this);
        if (BattalionData)
        {
            BattalionData->Data = Battalion;
            BattalionListView->AddItem(BattalionData);
        }
    }
}

