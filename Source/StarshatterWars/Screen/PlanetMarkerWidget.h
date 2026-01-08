#pragma once

#include "CoreMinimal.h"
#include "SystemMarkerWidget.h"
#include "../Game/GameStructs.h"
#include "PlanetMarkerWidget.generated.h"

class APlanetPanelActor;
class UImage;
class UBorder;
class UTextBlock;

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

protected:
	// These names MUST match your PlanetMarkerWidget BP widget variable names
	UPROPERTY(meta = (BindWidgetOptional)) UImage* PlanetImage = nullptr;
	UPROPERTY(meta = (BindWidgetOptional)) UBorder* HighlightBorder = nullptr;
	UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* PlanetNameText = nullptr;

	// Base widget hooks: these ensure the base uses PlanetImage (not BodyImage)
	virtual UImage* GetMarkerImage() const override { return PlanetImage; }
	virtual UBorder* GetMarkerHighlightBorder() const override { return HighlightBorder; }
	virtual UTextBlock* GetMarkerNameText() const override { return PlanetNameText; }

private:	
	UPROPERTY()
	UTextureRenderTarget2D* PlanetRT = nullptr;
};
