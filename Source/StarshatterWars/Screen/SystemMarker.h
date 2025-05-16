// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/GameStructs.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "../System/SSWGameInstance.h"
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
    UImage* StarImage;
    UPROPERTY(meta = (BindWidgetOptional))
    UImage* IffImage;
     UPROPERTY(meta = (BindWidgetOptional))
    class UTextBlock* SystemNameText;

    // Initialize with system data and available textures
    UFUNCTION()
    void Init(const FS_Galaxy& System);
    UTexture2D* LoadTextureFromFile(FString Path);
    FSlateBrush CreateBrushFromTexture(UTexture2D* Texture, FVector2D ImageSize);
protected:
    void NativeConstruct() override;
};
