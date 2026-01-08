// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "PlanetMarkerWidget.h"
#include "../Foundation/PlanetUtils.h"
#include "../Foundation/SystemMapUtils.h"
#include "../Actors/PlanetPanelActor.h"


void UPlanetMarkerWidget::InitFromPlanetActor(const FS_PlanetMap& Planet, APlanetPanelActor* PlanetActor)
{
	PlanetData = Planet;
	InitCommon(PlanetData.Name, PlanetData.Radius); 

	if (!PlanetImage || !PlanetActor || !PlanetWidgetMaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitFromPlanetActor: missing setup"));
		return;
	}

	SetWidgetRenderTarget(PlanetActor->GetRenderTarget());
	bIselected = false;
}

FReply UPlanetMarkerWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		OnPlanetClicked.Broadcast(PlanetData.Name);
		return FReply::Handled();
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
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
