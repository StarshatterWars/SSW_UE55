// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "PlanetMarkerWidget.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "../Foundation/PlanetUtils.h"
#include "../Foundation/SystemMapUtils.h"
#include "../Actors/PlanetPanelActor.h"
#include "../System/SSWGameInstance.h"

void UPlanetMarkerWidget::SetSelected(bool bSelected)
{
	// Optional highlight logic
}

void UPlanetMarkerWidget::SetMarkerMaterial(UMaterialInterface* PlanetMat)
{
	PlanetWidgetMaterial = PlanetMat;
}

void UPlanetMarkerWidget::InitFromPlanetActor(const FS_PlanetMap& Planet, APlanetPanelActor* PlanetActor)
{
	SetVisibility(ESlateVisibility::Visible);
	
	PlanetData = Planet;
	SetToolTipText(FText::FromString(PlanetData.Name));

	if (PlanetNameText)
	{
		PlanetNameText->SetText(FText::FromString(PlanetData.Name));
		PlanetNameText->SetColorAndOpacity(FLinearColor::White);
	}

	UTextureRenderTarget2D* RT = PlanetActor->GetRenderTarget();

	if (!PlanetImage || !PlanetActor || !PlanetWidgetMaterial || !RT)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitFromPlanetActor: missing setup"));
		return;
	}

	SetWidgetRenderTarget(RT);
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
