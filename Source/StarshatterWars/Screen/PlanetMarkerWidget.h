// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/GameStructs.h" // FS_Galaxy struct
#include "../System/SSWGameInstance.h"
#include "PlanetMarkerWidget.generated.h"

class UImage;
class UBorder;

/**
 * 
 */

DECLARE_DELEGATE_OneParam(FOnMarkerClicked, const FString&);

UCLASS()
class STARSHATTERWARS_API UPlanetMarkerWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// Called to assign the planet's display name
	void SetPlanetName(const FString& InName);

	// Called to highlight or un-highlight this marker
	void SetSelected(bool bSelected);

	// Optional: Retrieve name
	const FString& GetPlanetName() const { return PlanetName; }

	// Initialize with system data and available textures
    UFUNCTION()
    void Init(const FS_PlanetMap& System);

	FOnMarkerClicked OnClicked;

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* PlanetImage;

	UPROPERTY(meta = (BindWidgetOptional))
	UBorder* HighlightBorder;

	UPROPERTY(meta = (BindWidgetOptional))
    class UTextBlock* PlanetNameText;

	UTexture2D* LoadTextureFromFile(FString Path);
	FSlateBrush CreateBrushFromTexture(UTexture2D* Texture, FVector2D ImageSize);
	FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent);
private:
	FString PlanetName;
	FS_PlanetMap PlanetData;
};	

