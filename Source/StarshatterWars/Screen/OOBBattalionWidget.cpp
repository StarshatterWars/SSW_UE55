// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OOBBattalionWidget.h"

#include "Components/TextBlock.h"
//#include "OOBBatteryWidget.h"
//#include "OOBStarbaseWidget.h"
//#include "OOBStationWidget.h"

void UOOBBattalionWidget::NativeConstruct()
{
    Super::NativeConstruct();
    // Nothing needed here unless you want future custom animations
}

void UOOBBattalionWidget::SetData(const FS_OOBBattalion& InBattalion)
{
    Data = InBattalion;

    if (Label)
    {
        Label->SetText(FText::FromString(Data.Name));
    }

    Children.Empty(); // Reset children
}

void UOOBBattalionWidget::BuildChildren()
{
    Children.Empty();

    // Build Batteries
    /*for (const FS_OOBBattery& Battery : BattalionData.Battery)
    {
        UOOBBatteryItemWidget* BatteryItem = CreateWidget<UOOBBatteryItemWidget>(GetWorld(), UOOBBatteryItemWidget::StaticClass());
        if (BatteryItem)
        {
            BatteryItem->SetBatteryData(Battery);
            Children.Add(BatteryItem);
        }
    }

    // Build Starbases
    for (const FS_OOBStarbase& Starbase : BattalionData.Starbase)
    {
        UOOBStarbaseItemWidget* StarbaseItem = CreateWidget<UOOBStarbaseItemWidget>(GetWorld(), UOOBStarbaseItemWidget::StaticClass());
        if (StarbaseItem)
        {
            StarbaseItem->SetStarbaseData(Starbase);
            Children.Add(StarbaseItem);
        }
    }

    // Build Stations
    for (const FS_OOBStation& Station : BattalionData.Station)
    {
        UOOBStationItemWidget* StationItem = CreateWidget<UOOBStationItemWidget>(GetWorld(), UOOBStationItemWidget::StaticClass());
        if (StationItem)
        {
            StationItem->SetStationData(Station);
            Children.Add(StationItem);
        }
    }*/
}


