// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OOBAttackSquadronWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "OOBCarrierGroupItem.h"
#include "OOBAttackItem.h"
#include "OOBFighterUnit.h"
#include "OperationsScreen.h"
#include "Components/ListView.h" 

void UOOBAttackSquadronWidget::NativeConstruct()
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
}

void UOOBAttackSquadronWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    if (UOOBAttackItem* AttackData = Cast<UOOBAttackItem>(ListItemObject))
    {
        if (NameText)
        {
            NameText->SetText(FText::FromString(AttackData->Data.Name));
        }

        Data = AttackData->Data;

        // Expand/collapse setup
        bIsExpanded = false;

        if (ElementListView) ElementListView->ClearListItems();

        BuildChildren(AttackData->Data);
    }
}


FReply UOOBAttackSquadronWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    ToggleExpansion(); // << Expand or collapse when clicked
    USSWGameInstance* SSWInstance = Cast<USSWGameInstance>(GetGameInstance());
    SSWInstance->SetActiveUnit(true, Data.Name, Data.Empire, Data.Type, Data.Location);
    SSWInstance->bIsDisplayUnitChanged = true;
    return FReply::Handled();
}

void UOOBAttackSquadronWidget::ToggleExpansion()
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
}

void UOOBAttackSquadronWidget::BuildChildren(const FS_OOBAttack& AttackDataStruct)
{
    if (!ElementListView) {
        UE_LOG(LogTemp, Error, TEXT("Attack Squadron ListView is not valid!"));
        return;
    }

    ElementListView->ClearListItems();

    // Fill Units
    for (const FS_OOBFighterUnit& Element : AttackDataStruct.Unit)
    {
        UE_LOG(LogTemp, Error, TEXT("Fighter Found: %s"), *Element.Name);
        UOOBFighterUnit* FighterData = NewObject<UOOBFighterUnit>(this);
        if (FighterData)
        {
            FighterData->Data = Element;
            ElementListView->AddItem(FighterData);
        }
    }
}

