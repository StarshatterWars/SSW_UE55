// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OOBInterceptSquadronWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "OOBInterceptorItem.h"
#include "OOBFighterUnit.h"
#include "OperationsScreen.h"
#include "Components/ListView.h"

void UOOBInterceptSquadronWidget::NativeConstruct()
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

void UOOBInterceptSquadronWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    if (UOOBInterceptorItem* FighterData = Cast<UOOBInterceptorItem>(ListItemObject))
    {
        if (NameText)
        {
            NameText->SetText(FText::FromString(FighterData->Data.Name));
        }

        Data = FighterData->Data;
        // Expand/collapse setup
        bIsExpanded = false;

        if (ElementListView) ElementListView->ClearListItems();

        BuildChildren(FighterData->Data);
    }
}

FReply UOOBInterceptSquadronWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    ToggleExpansion(); // << Expand or collapse when clicked
    USSWGameInstance* SSWInstance = Cast<USSWGameInstance>(GetGameInstance());
    SSWInstance->SetActiveUnit(true, Data.Name, Data.Empire, Data.Type, Data.Location);
    SSWInstance->bIsDisplayUnitChanged = true;
    return FReply::Handled();
}

void UOOBInterceptSquadronWidget::ToggleExpansion()
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

void UOOBInterceptSquadronWidget::BuildChildren(const FS_OOBIntercept& FighterDataStruct)
{
    if (!ElementListView) {
        UE_LOG(LogTemp, Error, TEXT("Intercept Squadron ListView is not valid!"));
        return;
    }

    ElementListView->ClearListItems();

    // Fill Units
    for (const FS_OOBFighterUnit& Element : FighterDataStruct.Unit)
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


