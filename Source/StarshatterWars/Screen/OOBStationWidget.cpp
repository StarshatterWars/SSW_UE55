// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OOBStationWidget.h"
#include "OOBStationItem.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "OOBUnitItem.h"
#include "../Game/GameStructs.h" // FS_OOBFleet definition 

void UOOBStationWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UOOBStationWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    if (UOOBStationItem* StationData = Cast<UOOBStationItem>(ListItemObject))
    {
        if (NameText)
        {
            NameText->SetText(FText::FromString(StationData->Data.Name));
        }
    }
}

FReply UOOBStationWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    ShowElementData(); // << Expand or collapse when clicked
    return FReply::Handled();
}

void UOOBStationWidget::ShowElementData()
{
    // Show Data in Operations Screen
}
