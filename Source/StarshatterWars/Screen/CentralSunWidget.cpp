// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "CentralSunWidget.h"
#include "Components/Image.h"

void UCentralSunWidget::InitializeFromSunActor(ACentralSunActor* SunActor)
{
	if (!SunActor || !SunImage || !SunWidgetMaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitializeFromSunActor: missing data"));
		return;
	}

	UTextureRenderTarget2D* RenderTarget = SunActor->GetRenderTarget();
	
	if (!RenderTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("No render target found on SunActor."));
		return;
	}
	UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(SunWidgetMaterial, this);
	DynMat->SetTextureParameterValue("InputTexture", RenderTarget);

	// Apply the dynamic material to the image brush
	FSlateBrush Brush;
	Brush.SetResourceObject(DynMat);
	Brush.ImageSize = FVector2D(512.f, 512.f);
	SunImage->SetBrush(Brush);

	UE_LOG(LogTemp, Log, TEXT("CentralSunWidget initialized with render target: %s"), *RenderTarget->GetName());
}

