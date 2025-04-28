// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OOBElementWidget.h"
#include "OOBUnitItem.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "../Game/GameStructs.h" // FS_OOBFleet definition

void UOOBElementWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UOOBElementWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    if (UOOBUnitItem* UnitData = Cast<UOOBUnitItem>(ListItemObject))
    {
        if (NameText)
        {
            NameText->SetText(FText::FromString(UnitData->Data.Name));
        }
    }
}


FReply UOOBElementWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    ShowElementData(); // << Expand or collapse when clicked
    return FReply::Handled();
}

void UOOBElementWidget::ShowElementData()
{
    // Show Data in Operations Screen
}

