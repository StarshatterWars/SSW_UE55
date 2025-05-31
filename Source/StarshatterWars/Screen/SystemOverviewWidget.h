// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanelSlot.h"
#include "../Actors/SystemOverviewActor.h"
#include "Materials/MaterialInterface.h"
#include "SystemOverviewWidget.generated.h"


/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API USystemOverviewWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Set this from code before or after AddToViewport()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render")
	UTextureRenderTarget2D* OverviewRenderTarget;

	// Call after setting OverviewRenderTarget if created dynamically
	void UpdateOverviewImage(UTextureRenderTarget2D* RT);

	UFUNCTION(BlueprintCallable)
	void InitializeFromOverlayActor(ASystemOverviewActor* OverviewActor);

	// Material reference for black-transparent UI material
	UPROPERTY(EditDefaultsOnly, Category = "Render")
	UMaterialInterface* OverviewWidgetMaterial;

protected:
	void SetWidgetRenderTarget(UTextureRenderTarget2D* InRT);
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* OverviewImage;
	UPROPERTY()
	ASystemOverviewActor* OverlayActor;
};