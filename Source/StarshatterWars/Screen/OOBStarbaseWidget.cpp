// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OOBStarbaseWidget.h"
#include "OOBStarbaseItem.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "OOBUnitItem.h"
#include "../Game/GameStructs.h" // FS_OOBFleet definition

void UOOBStarbaseWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UOOBStarbaseWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    if (UOOBStarbaseItem* StarbaseData = Cast<UOOBStarbaseItem>(ListItemObject))
    {
        if (NameText)
        {
            NameText->SetText(FText::FromString(StarbaseData->Data.Name));
        }
    }
}

FReply UOOBStarbaseWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    ShowElementData(); // << Expand or collapse when clicked
    return FReply::Handled();
}

void UOOBStarbaseWidget::ShowElementData()
{
    // Show Data in Operations Screen
}

