// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Materials/MaterialInterface.h"
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
	void SetMoonName(const FString& InName);
	void SetSelected(bool bSelected);
	void SetMarkerMaterial(UMaterialInterface* MoonMat);
	const FString& GetMoonName() const { return MoonName; }

	UFUNCTION()
	void Init(const FS_MoonMap& Moon);

	UFUNCTION()
	void InitFromMoonActor(const FS_MoonMap& Planet, AMoonPanelActor* MoonActor);

	FOnMoonClicked OnMoonClicked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moon UI")
	UMaterialInterface* MoonWidgetMaterial;

protected:
	UPROPERTY(meta = (BindWidgetOptional)) UImage* MoonImage;
	UPROPERTY(meta = (BindWidgetOptional)) UBorder* HighlightBorder;
	UPROPERTY(meta = (BindWidgetOptional)) class UTextBlock* MoonNameText;

private:
	FString MoonName;
	FS_MoonMap MoonData;

	UTexture2D* LoadTextureFromFile(FString Path);
	FSlateBrush CreateBrushFromTexture(UTexture2D* Texture, FVector2D ImageSize);
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
};