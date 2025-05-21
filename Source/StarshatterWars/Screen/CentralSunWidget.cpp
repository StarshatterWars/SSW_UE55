// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "CentralSunWidget.h"
#include "Components/Image.h"

void UCentralSunWidget::InitializeFromSunActor(ACentralSunActor* SunActor)
{
	if (SunActor)
	{
		SetSunRenderWithMaterial(SunActor->GetRenderTarget()); // use material version
	}
}

void UCentralSunWidget::SetSunRender(UTextureRenderTarget2D* RenderTarget)
{
	if (SunImage && RenderTarget)
	{
		FSlateBrush Brush;
		Brush.SetResourceObject(static_cast<UObject*>(RenderTarget)); // <-- fix here
		Brush.ImageSize = FVector2D(512, 512);
		SunImage->SetBrush(Brush);
	}
}

void UCentralSunWidget::SetSunRenderWithMaterial(UTextureRenderTarget2D* RenderTarget)
{
	if (!SunImage || !BlackTransparentMaterial || !RenderTarget) return;

	// Create a dynamic material instance and set the texture
	UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(BlackTransparentMaterial, this);
	DynMat->SetTextureParameterValue(FName("InputTexture"), RenderTarget);
	DynMat->SetScalarParameterValue("ColorMultiplier", 10.0f);
	
	// Apply the dynamic material to the SunImage
	FSlateBrush Brush;
	Brush.SetResourceObject(DynMat);
	Brush.ImageSize = FVector2D(64, 64);
	SunImage->SetBrush(Brush);
}
