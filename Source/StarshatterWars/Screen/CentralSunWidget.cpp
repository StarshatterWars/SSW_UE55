// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "CentralSunWidget.h"
#include "Components/Image.h"
#include "../System/SSWGameInstance.h"

void UCentralSunWidget::InitializeFromSunActor(ACentralSunActor* SunActor)
{
	if (!SunActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitializeFromSunActor: missing SunActor"));
		return;
	}

	if (!SunImage)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitializeFromSunActor: missing SunImage"));
		return;
	}

	if (!SunWidgetMaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitializeFromSunActor: missing SunWidgetMaterial"));
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

	// Apply to image brush
	SunImage->SetBrushFromMaterial(DynMat);
	SunImage->SetBrushSize(FVector2D(64, 64)); // match RT size

	UE_LOG(LogTemp, Log, TEXT("CentralSunWidget initialized with render target: %s"), *RenderTarget->GetName());
}

FReply UCentralSunWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	UE_LOG(LogTemp, Log, TEXT("Sun clicked in CentralSunWidget"));
	OnSunClicked.Broadcast();
	return FReply::Handled();
}