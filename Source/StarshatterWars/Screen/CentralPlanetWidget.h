// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Materials/MaterialInterface.h"
#include "../Actors/PlanetPanelActor.h"
#include "../Game/GameStructs.h"
#include "CentralPlanetWidget.generated.h"

/**
 * 
 */

class UImage;
class UBorder;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectClicked, const FString&, PlanetName);

UCLASS()
class STARSHATTERWARS_API UCentralPlanetWidget : public UUserWidget
{
	GENERATED_BODY()
	
	public:
	UPROPERTY()
	FS_PlanetMap PlanetData;
	UFUNCTION()
	void SetSelected(bool bSelected);
	
	UFUNCTION()void SetMarkerMaterial(UMaterialInterface* PlanetMat);

	UFUNCTION()
	void SetWidgetRenderTarget(UTextureRenderTarget2D* InRT);

	UFUNCTION()
	void InitFromPlanetActor(const FS_PlanetMap& Planet, APlanetPanelActor* PlanetActor);

	FOnObjectClicked OnObjectClicked;

	UPROPERTY()
	UMaterialInterface* PlanetWidgetMaterial;

	UPROPERTY()
	bool bIselected = false;

protected:
	UPROPERTY(meta = (BindWidgetOptional)) UImage* PlanetImage;
	UPROPERTY(meta = (BindWidgetOptional)) UBorder* HighlightBorder;
	UPROPERTY(meta = (BindWidgetOptional)) class UTextBlock* PlanetNameText;

private:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	
	UPROPERTY()
	UTextureRenderTarget2D* PlanetRT = nullptr;

};

	
	

