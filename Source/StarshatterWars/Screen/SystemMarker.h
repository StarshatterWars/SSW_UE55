// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/GameStructs.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "SystemMarker.generated.h"

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API USystemMarker : public UUserWidget
{
	GENERATED_BODY()
	
	public:
    UPROPERTY(meta = (BindWidgetOptional))
    class UImage* StarImage;
    UPROPERTY(meta = (BindWidgetOptional))
    class UImage* IffImage;
     UPROPERTY(meta = (BindWidgetOptional))
    class UTextBlock* SystemNameText;

    // Initialize with system data and available textures
    UFUNCTION()
    void Init(const FS_Galaxy& System, const TMap<FString, UTexture2D*>& StarTextures);
};
