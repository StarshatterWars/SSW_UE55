// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OOBFighterElementWidget.h"
#include "OOBFighterUnit.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "../Game/GameStructs.h" // FS_OOBFleet definition

void UOOBFighterElementWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UOOBFighterElementWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    if (UOOBFighterUnit* UnitData = Cast<UOOBFighterUnit>(ListItemObject))
    {
        if (NameText)
        {
            NameText->SetText(FText::FromString(FString::Printf(TEXT("%i x %s"), UnitData->Data.Count, *UnitData->Data.Design)));
        }
    }
}

FReply UOOBFighterElementWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    ShowElementData(); // << Expand or collapse when clicked
    return FReply::Handled();
}

void UOOBFighterElementWidget::ShowElementData()
{
    // Show Data in Operations Screen
}





