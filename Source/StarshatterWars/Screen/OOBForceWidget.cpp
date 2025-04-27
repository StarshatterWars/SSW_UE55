// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OOBForceWidget.h"
#include "OOBFleetItem.h"
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

    if (NameText)
    {
        /*if (UHorizontalBoxSlot* HBoxSlot = Cast<UHorizontalBoxSlot>(NameText->Slot))
        {
            const float IndentSize = 20.0f;
            HBoxSlot->SetPadding(FMargin(IndentLevel * IndentSize, 0.0f, 0.0f, 0.0f));
        }*/
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


void UOOBForceWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    NameText->SetText(FText::FromString(Data.Name));
}

void UOOBForceWidget::SetForceData(const FS_OOBForce& InForce, int32 InIndentLevel)
{
    ForceData = InForce;
    IndentLevel = InIndentLevel;

    UE_LOG(LogTemp, Log, TEXT("Setting Force Name: %s"), *InForce.Name);

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
    if (!FleetListView) return;

    FleetListView->ClearListItems();

    for (const FS_OOBFleet& Fleet : ForceData.Fleet)
    {
        // 1. Create a Data Object (not a widget)
        UOOBFleetItem* FleetData = NewObject<UOOBFleetItem>(this);
        if (FleetData)
        {
            // 2. Fill the Data Object from the FS_OOBFleet info
            FleetData->Data = Fleet;

            // 3. Add the DataObject to the ListView
            FleetListView->AddItem(FleetData); // <- Unreal will auto-create a widget!
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