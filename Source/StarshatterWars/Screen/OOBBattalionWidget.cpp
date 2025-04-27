// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OOBBattalionWidget.h"

#include "Components/TextBlock.h"
#include "OOBBatteryWidget.h"
#include "OOBStarbaseWidget.h"
#include "OOBStationWidget.h"
#include "Components/Image.h"
#include "Components/HorizontalBoxSlot.h" 

void UOOBBattalionWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    if (NameText)
    {
        if (UHorizontalBoxSlot* HBoxSlot = Cast<UHorizontalBoxSlot>(NameText->Slot))
        {
            const float IndentSize = 20.0f;
            HBoxSlot->SetPadding(FMargin(IndentLevel * IndentSize, 0.0f, 0.0f, 0.0f));
        }
    }

    if (ExpandIcon)
    {
        ExpandIcon->SetVisibility(ESlateVisibility::Visible);

        if (bIsExpanded)
        {
            ExpandIcon->SetBrushFromTexture(ExpandedIconTexture); // Expanded
        }
        else
        {
            ExpandIcon->SetBrushFromTexture(CollapsedIconTexture); // Collapsed
        }
    }
}

void UOOBBattalionWidget::SetData(const FS_OOBBattalion& InBattalion)
{
    Data = InBattalion;

    if (NameText)
    {
        NameText->SetText(FText::FromString(Data.Name));
    }

    Children.Empty(); // Reset children
}

void UOOBBattalionWidget::BuildChildren()
{
    Children.Empty();

    // Build Batteries
    for (const FS_OOBBattery& Battery : Data.Battery)
    {
        UOOBBatteryWidget* BatteryItem = CreateWidget<UOOBBatteryWidget>(GetWorld(), UOOBBatteryWidget::StaticClass());
        if (BatteryItem)
        {
            BatteryItem->SetData(Battery, 2);
            Children.Add(BatteryItem);
        }
    }

    // Build Starbases
    for (const FS_OOBStarbase& Starbase : Data.Starbase)
    {
        UOOBStarbaseWidget* StarbaseItem = CreateWidget<UOOBStarbaseWidget>(GetWorld(), UOOBStarbaseWidget::StaticClass());
        if (StarbaseItem)
        {
            StarbaseItem->SetData(Starbase, 2);
            Children.Add(StarbaseItem);
        }
    }

    // Build Stations
    for (const FS_OOBStation& Station : Data.Station)
    {
        UOOBStationWidget* StationItem = CreateWidget<UOOBStationWidget>(GetWorld(), UOOBStationWidget::StaticClass());
        if (StationItem)
        {
            StationItem->SetData(Station, 2);
            Children.Add(StationItem);
        }
    }
}


