// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "CentralPlanetWidget.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "../Foundation/PlanetUtils.h"
#include "../Foundation/SystemMapUtils.h"
#include "../Actors/CentralPlanetActor.h"
#include "../System/SSWGameInstance.h"

void UCentralPlanetWidget::SetSelected(bool bSelected)
{
	// Optional highlight logic
}

void UCentralPlanetWidget::SetMarkerMaterial(UMaterialInterface* PlanetMat)
{
	PlanetWidgetMaterial = PlanetMat;
}

void UCentralPlanetWidget::InitFromPlanetActor(const FS_PlanetMap& Planet, ACentralPlanetActor* PlanetActor)
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
	bIselected = false;
}

FReply UCentralPlanetWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		OnCentralPlanetClicked.Broadcast(PlanetData.Name);
		return FReply::Handled();
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UCentralPlanetWidget::SetWidgetRenderTarget(UTextureRenderTarget2D* InRT)
{
	if (InRT && PlanetImage && PlanetWidgetMaterial)
	{
		float SizePx = SystemMapUtils::GetUISizeFromRadius(PlanetData.Radius) / 2;
		SystemMapUtils::ApplyRenderTargetToImage(
			this,
			PlanetImage,
			PlanetWidgetMaterial,
			InRT,
			FVector2D(SizePx, SizePx)
		);
	}
}


