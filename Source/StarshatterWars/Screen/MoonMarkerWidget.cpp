// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "MoonMarkerWidget.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "../Foundation/PlanetUtils.h"
#include "../Foundation/MoonUtils.h"
#include "../Foundation/SystemMapUtils.h"
#include "../Actors/PlanetPanelActor.h"
#include "../System/SSWGameInstance.h"

void UMoonMarkerWidget::SetSelected(bool bSelected)
{
	// Optional highlight logic
}

void UMoonMarkerWidget::InitFromMoonActor(const FS_MoonMap& Moon, AMoonPanelActor* MoonActor)
{
	MoonData = Moon;
	SetToolTipText(FText::FromString(Moon.Name));

	if (MoonNameText)
	{
		MoonNameText->SetText(FText::FromString(MoonData.Name));
		MoonNameText->SetColorAndOpacity(FLinearColor::White);
	}

	if (!MoonImage || !MoonActor || !MoonWidgetMaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitFromMoonActor: missing setup"));
		return;
	}

	SetWidgetRenderTarget(MoonActor->GetRenderTarget());
}

FReply UMoonMarkerWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		OnMoonClicked.Broadcast(MoonData.Name);
		return FReply::Handled();
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UMoonMarkerWidget::SetWidgetRenderTarget(UTextureRenderTarget2D* InRT)
{
	if (InRT && MoonImage && MoonWidgetMaterial)
	{
		float SizePx = MoonUtils::GetUISizeFromRadius(MoonData.Radius) / 2;
		SystemMapUtils::ApplyRenderTargetToImage(
			this,
			MoonImage,
			MoonWidgetMaterial,
			InRT,
			FVector2D(SizePx, SizePx)
		);
	}
}



