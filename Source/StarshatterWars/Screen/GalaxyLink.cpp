// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "GalaxyLink.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"


void UGalaxyLink::ConfigureLine(float Length, float AngleDegrees, FLinearColor Tint /*= FLinearColor::White*/)
{
    if (!LineImage) return;

    LineImage->SetRenderTransformAngle(AngleDegrees);
    LineImage->SetColorAndOpacity(Tint);

    if (UCanvasPanelSlot* ImageSlot = Cast<UCanvasPanelSlot>(LineImage->Slot))
    {
        ImageSlot->SetSize(FVector2D(Length/2, 4)); // Set line size only here
        //ImageSlot->SetAlignment(FVector2D(0.f, 0.5f));
    }

    // Set fixed widget size to accommodate rotation (optional)
    if (UCanvasPanelSlot* RootSlot = Cast<UCanvasPanelSlot>(Slot))
    {
        RootSlot->SetAutoSize(true); // or set size if needed
    }
}
