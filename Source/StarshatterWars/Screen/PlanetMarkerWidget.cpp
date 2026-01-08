// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "PlanetMarkerWidget.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "../Foundation/PlanetUtils.h"
#include "../Foundation/SystemMapUtils.h"
#include "../Actors/PlanetPanelActor.h"
#include "../System/SSWGameInstance.h"

void UPlanetMarkerWidget::InitFromPlanetActor(const FS_PlanetMap& Planet, APlanetPanelActor* PlanetActor)
{
	PlanetData = Planet;
	UTextureRenderTarget2D* RT = PlanetActor ? PlanetActor->GetRenderTarget() : nullptr;

	InitCommon(PlanetData.Name, PlanetData.Radius);

	if (!PlanetImage || !PlanetActor || !PlanetWidgetMaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitFromPlanetActor: missing setup"));
		return;
	}

	SetWidgetRenderTarget(PlanetActor->GetRenderTarget());
}


void UPlanetMarkerWidget::SetWidgetRenderTarget(UTextureRenderTarget2D* InRT)
{
	if (InRT && PlanetImage && PlanetWidgetMaterial)
	{
		float SizePx = PlanetUtils::GetUISizeFromRadius(PlanetData.Radius) / 2;
		SystemMapUtils::ApplyRenderTargetToImage(
			this,
			PlanetImage,
			PlanetWidgetMaterial,
			InRT,
			FVector2D(SizePx, SizePx)
		);
	}
}
