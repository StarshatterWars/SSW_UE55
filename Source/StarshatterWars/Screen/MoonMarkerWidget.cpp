// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "MoonMarkerWidget.h"

void UMoonMarkerWidget::InitFromMoonActor(const FS_MoonMap& Moon, AMoonPanelActor* MoonActor)
{
	MoonData = Moon;
	bSelected = false;

	InitCommon(MoonData.Name, MoonData.Radius);

	if (!ObjectImage || !MoonActor || !MoonWidgetMaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitFromMoonActor: missing setup"));
		return;
	}

	SetWidgetRenderTarget(MoonActor->GetRenderTarget(), MoonWidgetMaterial);
}

