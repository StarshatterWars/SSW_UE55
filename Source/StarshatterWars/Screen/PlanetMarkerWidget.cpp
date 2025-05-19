// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "PlanetMarkerWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UPlanetMarkerWidget::SetPlanetName(const FString& InName)
{
	PlanetName = InName;
	SetToolTipText(FText::FromString(InName));
}

void UPlanetMarkerWidget::SetSelected(bool bSelected)
{
	if (PlanetImage)
	{
		PlanetImage->SetColorAndOpacity(bSelected ? FLinearColor::Yellow : FLinearColor::White);
	}
}