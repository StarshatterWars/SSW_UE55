// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OOBBatteryWidget.h"
#include "OOBBatteryItem.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "OperationsScreen.h"
#include "../Game/GameStructs.h" // FS_OOBFleet definition

void UOOBBatteryWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UOOBBatteryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    if (UOOBBatteryItem* BatteryData = Cast<UOOBBatteryItem>(ListItemObject))
    {
        if (NameText)
        {
            NameText->SetText(FText::FromString(BatteryData->Data.Name));
        }

        Data = BatteryData->Data;
    }
}

FReply UOOBBatteryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    ShowUnitData();
    return FReply::Handled();
}

void UOOBBatteryWidget::ShowUnitData()
{
    USSWGameInstance* SSWInstance = Cast<USSWGameInstance>(GetGameInstance());
    SSWInstance->SetActiveUnit(true, Data.Name, Data.Empire, Data.Type, Data.Location);
    SSWInstance->bIsDisplayUnitChanged = true;
}

