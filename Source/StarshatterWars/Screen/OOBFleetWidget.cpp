// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OOBFleetWidget.h"
#include "Components/TextBlock.h"
#include "OOBBattleWidget.h"
#include "OOBDesronWidget.h"
#include "OOBCarrierWidget.h"

void UOOBFleetWidget::NativeConstruct()
{
    Super::NativeConstruct();
    // Nothing needed here yet
}

void UOOBFleetWidget::SetData(const FS_OOBFleet& InFleet)
{
    Data = InFleet;

    if (Label)
    {
        Label->SetText(FText::FromString(Data.Name));
    }

    Children.Empty();
}

void UOOBFleetWidget::BuildChildren()
{
    Children.Empty();

    // Build Battle Groups
    for (const FS_OOBBattle& Battle : Data.Battle)
    {
        UOOBBattleWidget* BattleItem = CreateWidget<UOOBBattleWidget>(GetWorld(), UOOBBattleWidget::StaticClass());
        if (BattleItem)
        {
            BattleItem->SetData(Battle);
            Children.Add(BattleItem);
        }
    }

    // Build Carriers
    for (const FS_OOBCarrier& Carrier : Data.Carrier)
    {
        UOOBCarrierWidget* CarrierItem = CreateWidget<UOOBCarrierWidget>(GetWorld(), UOOBCarrierWidget::StaticClass());
        if (CarrierItem)
        {
            CarrierItem->SetData(Carrier);
            Children.Add(CarrierItem);
        }
    }

    // Build Destroyer Squadrons
    for (const FS_OOBDestroyer& Desron : Data.Destroyer)
    {
        UOOBDesronWidget* DesronItem = CreateWidget<UOOBDesronWidget>(GetWorld(), UOOBDesronWidget::StaticClass());
        if (DesronItem)
        {
            DesronItem->SetData(Desron);
            Children.Add(DesronItem);
        }
    }
}


