/*  Project nGenEx
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.
	SUBSYSTEM:    SSW	
	FILE:         PlanetMarkerWidget.cpp
	AUTHOR:       Carlos Bott
*/


#include "PlanetMarkerWidget.h"
#include "../Actors/PlanetPanelActor.h"


void UPlanetMarkerWidget::InitFromPlanetActor(const FS_PlanetMap& Planet, APlanetPanelActor* PlanetActor)
{
	PlanetData = Planet;
	bSelected = false;

	InitCommon(PlanetData.Name, PlanetData.Radius); 

	if (!ObjectImage || !PlanetActor || !ObjectWidgetMaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitFromPlanetActor: missing setup"));
		return;
	}

	SetWidgetRenderTarget(PlanetActor->GetRenderTarget(), ObjectWidgetMaterial);
}

