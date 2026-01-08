// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "SystemMarkerWidget.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanelSlot.h"
#include "Materials/MaterialInterface.h"
#include "../Actors/PlanetPanelActor.h"
#include "PlanetMarkerWidget.generated.h"

class UImage;
class UBorder;

UCLASS()
class STARSHATTERWARS_API UPlanetMarkerWidget : public USystemMarkerWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	FS_PlanetMap PlanetData;

	UFUNCTION()
	void SetWidgetRenderTarget(UTextureRenderTarget2D* InRT);

	UFUNCTION()
	void InitFromPlanetActor(const FS_PlanetMap& Planet, APlanetPanelActor* PlanetActor);

	UPROPERTY()
	UMaterialInterface* PlanetWidgetMaterial;	

private:
	UPROPERTY()
	UTextureRenderTarget2D* PlanetRT = nullptr;

};
