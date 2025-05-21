// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/Image.h"
#include "../Actors/CentralSunActor.h"
#include "Materials/MaterialInterface.h"
#include "CentralSunWidget.generated.h"

/**
 * Widget that displays the 3D-rendered central star
 */

UCLASS()
class STARSHATTERWARS_API UCentralSunWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* SunImage;

	// Material reference for black-transparent UI material
	UPROPERTY(EditDefaultsOnly, Category = "Render")
	UMaterialInterface* BlackTransparentMaterial;
	
	UFUNCTION(BlueprintCallable)
	void InitializeFromSunActor(ACentralSunActor* SunActor);
	
	UFUNCTION(BlueprintCallable)
	void SetSunRender(UTextureRenderTarget2D* RenderTarget);
	
	UFUNCTION(BlueprintCallable)
	void SetSunRenderWithMaterial(UTextureRenderTarget2D* RenderTarget);
};
