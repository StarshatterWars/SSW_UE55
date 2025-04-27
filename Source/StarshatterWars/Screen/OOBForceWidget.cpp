// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OOBForceWidget.h"
#include "OOBForceItem.h"
#include "Components/TextBlock.h"
#include "../Game/GameStructs.h" // FS_OOBFleet definition
#include "OOBFleetWidget.h"
#include "OOBBattalionWidget.h"

void UOOBForceWidget::NativeConstruct()
{
    Super::NativeConstruct();
    // Nothing else needed here
}

void UOOBForceWidget::SetData(UOOBForceItem* InForceObject)
{
    Object = InForceObject;

    if (Object && Label)
    {
        Label->SetText(FText::FromString(Object->Data.Name));
    }

    Children.Empty(); // Reset children
}

void UOOBForceWidget::BuildChildren()
{
    Children.Empty();

    if (!Object) return;

    // Build Fleets
    for (const FS_OOBFleet& Fleet : Object->Data.Fleet)
    {
        UOOBFleetWidget* FleetItem = CreateWidget<UOOBFleetWidget>(GetWorld(), UOOBFleetWidget::StaticClass());
        if (FleetItem)
        {
            FleetItem->SetData(Fleet);
            Children.Add(FleetItem);
        }
    }

    // Build Battalions
    for (const FS_OOBBattalion& Battalion : Object->Data.Battalion)
    {
        UOOBBattalionWidget* BattalionItem = CreateWidget<UOOBBattalionWidget>(GetWorld(), UOOBBattalionWidget::StaticClass());
        if (BattalionItem)
        {
            BattalionItem->SetData(Battalion);
            Children.Add(BattalionItem);
        }
    }
}

