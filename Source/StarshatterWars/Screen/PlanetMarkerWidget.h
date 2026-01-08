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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlanetClicked, const FString&, PlanetName);

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

	FOnPlanetClicked OnPlanetClicked;

	UPROPERTY()
	UMaterialInterface* PlanetWidgetMaterial;	

private:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	
	UPROPERTY()
	UTextureRenderTarget2D* PlanetRT = nullptr;

};
