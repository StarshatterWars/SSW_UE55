// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Materials/MaterialInterface.h"
#include "PlanetMarkerwidget.h"
#include "../Actors/MoonPanelActor.h"
#include "MoonMarkerWidget.generated.h"

/**
 * 
 */

class UImage;
class UBorder;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMoonClicked, const FString&, MoonName);

UCLASS()
class STARSHATTERWARS_API UMoonMarkerWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	FS_MoonMap MoonData;
	
	void SetSelected(bool bSelected);
	void SetWidgetRenderTarget(UTextureRenderTarget2D* InRT);

	UFUNCTION()
	void InitFromMoonActor(const FS_MoonMap& Moon, AMoonPanelActor* MoonActor);

	FOnMoonClicked OnMoonClicked;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* MoonWidgetMaterial;

protected:
	UPROPERTY(meta = (BindWidgetOptional)) UImage* MoonImage;
	UPROPERTY(meta = (BindWidgetOptional)) UBorder* HighlightBorder;
	UPROPERTY(meta = (BindWidgetOptional)) class UTextBlock* MoonNameText;

private:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
};