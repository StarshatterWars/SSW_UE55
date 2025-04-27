// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OOBFleetWidget.h"
#include "Components/TextBlock.h"
#include "OOBBattleWidget.h"
#include "OOBDesronWidget.h"
#include "OOBCarrierWidget.h"
#include "Components/Image.h"
#include "OOBFleetItem.h"
#include "OperationsScreen.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ListView.h"

void UOOBFleetWidget::NativeConstruct()
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

    if (ChildListView)
    {
        ChildListView->ClearListItems(); // Clear existing items
    }
}


void UOOBFleetWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    NameText->SetText(FText::FromString(Data.Name));
}

void UOOBFleetWidget::SetFleetData(const FS_OOBFleet& InFleet, int32 InIndentLevel)
{
    Data = InFleet;
    IndentLevel = InIndentLevel;

    if (NameText)
    {
        NameText->SetText(FText::FromString(Data.Name));
    }

    if (ChildListView)
    {
        ChildListView->ClearListItems(); // Clear before repopulating
    }

    //BuildChildren();
}

void UOOBFleetWidget::BuildChildren()
{
    Children.Empty();

    // Add BattleGroups
    for (const FS_OOBBattle& Battle : Data.Battle)
    {
        UOOBBattleWidget* BattleItem = CreateWidget<UOOBBattleWidget>(GetWorld(), UOOBBattleWidget::StaticClass());
        if (BattleItem)
        {
            BattleItem->SetData(Battle, IndentLevel + 1);
            ChildListView->AddItem(BattleItem);
            Children.Add(BattleItem);
        }
    }

    // Add Carriers
    for (const FS_OOBCarrier& Carrier : Data.Carrier)
    {
        UOOBCarrierWidget* CarrierItem = CreateWidget<UOOBCarrierWidget>(GetWorld(), UOOBCarrierWidget::StaticClass());
        if (CarrierItem)
        {
            CarrierItem->SetData(Carrier, IndentLevel + 1);
            ChildListView->AddItem(CarrierItem);
            Children.Add(CarrierItem);
        }
    }

    // Add DesRons (Destroyer Squadrons)
    for (const FS_OOBDestroyer& Desron : Data.Destroyer)
    {
        UOOBDesronWidget* DesronItem = CreateWidget<UOOBDesronWidget>(GetWorld(), UOOBDesronWidget::StaticClass());
        if (DesronItem)
        {
            DesronItem->SetData(Desron, IndentLevel + 1);
            ChildListView->AddItem(DesronItem);
            Children.Add(DesronItem);
        }
    }
}

void UOOBFleetWidget::ToggleExpansion()
{
    bIsExpanded = !bIsExpanded;

    if (ExpandIcon)
    {
        if (bIsExpanded)
        {
            ExpandIcon->SetBrushFromTexture(ExpandedIconTexture); // Expanded (-)
        }
        else
        {
            ExpandIcon->SetBrushFromTexture(CollapsedIconTexture); // Collapsed (+)
        }
    }

    if (ChildListView)
    {
        ChildListView->SetVisibility(bIsExpanded ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}
