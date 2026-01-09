// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "SystemMarkerWidget.h"
#include "../Actors/MoonPanelActor.h"
#include "MoonMarkerWidget.generated.h"

class UImage;
class UBorder;

UCLASS()
class STARSHATTERWARS_API UMoonMarkerWidget : public USystemMarkerWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	FS_MoonMap MoonData;

	UFUNCTION()
	void InitFromMoonActor(const FS_MoonMap& Planet, AMoonPanelActor* MoonActor);

private:
	UPROPERTY()
	UTextureRenderTarget2D* MoonRT = nullptr;
};