// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "SystemOverviewWidget.h"

void USystemOverviewWidget::InitializeFromOverlayActor(ASystemOverviewActor* OA)
{
	OverlayActor = OA;

	UTextureRenderTarget2D* RT = OverlayActor->GetRenderTarget();

	if (!OverviewImage || !OverlayActor || !OverviewWidgetMaterial || !RT)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitializeFromOverlayActor: missing setup"));
		return;
	}

	SetWidgetRenderTarget(RT);
}


void USystemOverviewWidget::UpdateOverviewImage(UTextureRenderTarget2D* RT)
{
	// Load the UI-compatible material
	static UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(
		nullptr, TEXT("/Game/Screens/Operations/Widgets/M_SystemOverview_RT.M_SystemOverview_RT"));

	if (BaseMaterial && OverviewImage && RT)
	{
		UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		DynMat->SetTextureParameterValue("BaseTexture", RT);

		OverviewImage->SetBrushFromMaterial(DynMat);
	}
}

void USystemOverviewWidget::SetWidgetRenderTarget(UTextureRenderTarget2D* InRT)
{
	if (InRT && OverviewImage && OverviewWidgetMaterial)
	{
		SystemMapUtils::ApplyRenderTargetToImage(
			this,
			OverviewImage,
			OverviewWidgetMaterial,
			InRT,
			FVector2D(2048, 2048)
		);
	}
}

