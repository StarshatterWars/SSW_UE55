// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OOBElementWidget.h"
#include "OOBUnitItem.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "OperationsScreen.h"
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
            NameText->SetText(FText::FromString(UnitData->Data.DisplayName));
        }
        Data = UnitData->Data;
    }
}


FReply UOOBElementWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    ShowElementData(); // << Expand or collapse when clicked
    return FReply::Handled();
}

void UOOBElementWidget::ShowElementData()
{
    USSWGameInstance* SSWInstance = Cast<USSWGameInstance>(GetGameInstance());
    SSWInstance->SetActiveElement(true, Data.DisplayName, Data.Empire, Data.Type, Data.Location);
    SSWInstance->bIsDisplayElementChanged = true;
}

