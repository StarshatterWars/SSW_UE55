// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "CentralSunWidget.h"
#include "Components/Image.h"
#include "Components/Widget.h"
#include "Components/CanvasPanelSlot.h"
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
	const float Radius = SunActor->GetRadius();

	constexpr float MinRadius = 1.2e9f;
	constexpr float MaxRadius = 2.2e9f;
	constexpr float MinSize = 32.f;
	constexpr float MaxSize = 128.f;

	float Normalized = FMath::Clamp((Radius - MinRadius) / (MaxRadius - MinRadius), 0.f, 1.f);
	float SizePx = FMath::Lerp(MinSize, MaxSize, Normalized);

	SunImage->SetBrushSize(FVector2D(SizePx, SizePx));

	if (SunSlot)
	{
		SunSlot->SetSize(FVector2D(SizePx, SizePx));
	}

	UE_LOG(LogTemp, Log, TEXT("CentralSunWidget initialized with render target: %s, radius: %.2e km -> %.1f px"),
		*GetNameSafe(RenderTarget), Radius, SizePx);
	
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