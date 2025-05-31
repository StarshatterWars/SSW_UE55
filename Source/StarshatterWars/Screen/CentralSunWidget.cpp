// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "CentralSunWidget.h"
#include "Components/Image.h"
#include "Components/Widget.h"
#include "Components/CanvasPanelSlot.h"
#include "../System/SSWGameInstance.h"
#include "../Foundation/StarUtils.h"
#include "../Foundation/SystemMapUtils.h"

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

	CentralStar = SunActor;

	if (SunNameText) {
		SunNameText->SetText(FText::FromString(CentralStar->StarName));
	}

	UTextureRenderTarget2D* RT = CentralStar->GetRenderTarget();
	
	if (!RT)
	{
		UE_LOG(LogTemp, Warning, TEXT("No render target found on SunActor."));
		return;
	}
	StarSize = StarUtils::GetUISizeFromRadius(CentralStar->GetRadius()) / 2;
	SetWidgetRenderTarget(RT);
	
	float Scale = StarSize / 64.f; // assuming 64 is base size
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

void UCentralSunWidget::SetWidgetRenderTarget(UTextureRenderTarget2D* InRT)
{
	if (InRT && SunImage && SunWidgetMaterial)
	{
		SystemMapUtils::ApplyRenderTargetToImage(
			this,
			SunImage,
			SunWidgetMaterial,
			InRT,
			FVector2D(StarSize, StarSize)
		);
	}
}
