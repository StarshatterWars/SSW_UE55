// Project nGenEx
// Fractal Dev Games
// Copyright (C) 2024. All Rights Reserved.
// 
// SUBSYSTEM:    SSW
// FILE:         MoonMarkerWidget.cpp
// AUTHOR:       Carlos Bott


#include "MoonMarkerWidget.h"

void UMoonMarkerWidget::InitFromMoonActor(const FS_MoonMap& Moon, AMoonPanelActor* MoonActor)
{
	MoonData = Moon;
	bSelected = false;

	InitCommon(MoonData.Name, MoonData.Radius);

	if (!ObjectImage || !MoonActor || !ObjectWidgetMaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitFromMoonActor: missing setup"));
		return;
	}

	UTextureRenderTarget2D* RT = MoonActor->GetRenderTarget();

	UE_LOG(LogTemp, Warning, TEXT("Moon Init: Moon=%s Actor=%s RT=%s"),
		*MoonData.Name,
		*MoonActor->GetName(),
		RT ? *RT->GetName() : TEXT("NULL"));

	if (!RT)
	{
		UE_LOG(LogTemp, Warning, TEXT("Moon Init: RT is NULL (init called too early)"));
		return;
	}

		SetWidgetRenderTarget(RT, ObjectWidgetMaterial,  EBodyUISizeClass::Moon);
}

