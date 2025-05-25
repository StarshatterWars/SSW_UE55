// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Materials/MaterialInterface.h"
#include "../Actors/PlanetPanelActor.h"
#include "PlanetMarkerWidget.generated.h"

class UImage;
class UBorder;

DECLARE_DELEGATE_OneParam(FOnMarkerClicked, const FString&);

UCLASS()
class STARSHATTERWARS_API UPlanetMarkerWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetPlanetName(const FString& InName);
	void SetSelected(bool bSelected);
	void SetMarkerMaterial(UMaterialInterface* PlanetMat);
	const FString& GetPlanetName() const { return PlanetName; }

	UFUNCTION()
	void Init(const FS_PlanetMap& Planet);

	UFUNCTION()
	void InitFromPlanetActor(const FS_PlanetMap& Planet, APlanetPanelActor* PlanetActor);

	FOnMarkerClicked OnClicked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet UI")
	UMaterialInterface* PlanetWidgetMaterial;

protected:
	UPROPERTY(meta = (BindWidgetOptional)) UImage* PlanetImage;
	UPROPERTY(meta = (BindWidgetOptional)) UBorder* HighlightBorder;
	UPROPERTY(meta = (BindWidgetOptional)) class UTextBlock* PlanetNameText;

private:
	FString PlanetName;
	FS_PlanetMap PlanetData;

	UTexture2D* LoadTextureFromFile(FString Path);
	FSlateBrush CreateBrushFromTexture(UTexture2D* Texture, FVector2D ImageSize);
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
};
