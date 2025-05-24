// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "CentralSunWidget.h"
#include "Components/Image.h"
#include "Components/Widget.h"
#include "Components/CanvasPanelSlot.h"
#include "../System/SSWGameInstance.h"
#include "../Foundation/StarUtils.h"

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
	float SizePx = StarUtils::GetUISizeFromRadius(SunActor->GetRadius());

	// Set brush and visual size
	SunImage->SetBrushSize(FVector2D(SizePx, SizePx));

	if (UCanvasPanelSlot* ImageSlot = Cast<UCanvasPanelSlot>(SunImage->Slot))
	{
		ImageSlot->SetSize(FVector2D(SizePx, SizePx));
	}

	UE_LOG(LogTemp, Log, TEXT("CentralSunWidget initialized with render target: %s, radius: %.2e km -> %.1f px"),
		*GetNameSafe(RenderTarget), SunActor->GetRadius(), SizePx);
	
	float Scale = SizePx / 64.f; // assuming 64 is base size
	FWidgetTransform Transform;
	Transform.Scale = FVector2D(Scale, Scale);
	SunImage->SetRenderTransform(Transform);
}

FReply UCentralSunWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	UE_LOG(LogTemp, Log, TEXT("Sun clicked in CentralSunWidget"));
	OnSunClicked.Broadcast();
	return FReply::Handled();
}