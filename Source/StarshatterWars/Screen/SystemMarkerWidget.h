#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/TextureRenderTarget2D.h"
#include "SystemMarkerWidget.generated.h"

class UImage;
class UBorder;
class UTextBlock;
class UMaterialInterface;

/**
 * Base marker widget for bodies rendered via RT + MID.
 * Shared responsibilities:
 * - Ensure widget is visible on init
 * - Set tooltip + name text
 * - Apply RT to Image using SystemMapUtils::ApplyRenderTargetToImage
 * - Handle LMB click and call a virtual hook for derived broadcast
 */
UCLASS(Abstract)
class STARSHATTERWARS_API USystemMarkerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Call from derived InitFromXActor after it sets its data fields.
	void InitCommon(const FString& DisplayName, float Radius, UTextureRenderTarget2D* RenderTarget);

	// Optional selection hook (shared highlight border)
	UFUNCTION()
	virtual void SetSelected(bool bSelected);

protected:
	UPROPERTY(meta = (BindWidgetOptional)) UImage* BodyImage;

	virtual UImage* GetMarkerImage() const PURE_VIRTUAL(USystemMarkerWidget::GetMarkerImage, return nullptr;);
	virtual UTextBlock* GetMarkerNameText() const PURE_VIRTUAL(USystemBodyMarkerWidget::GetMarkerNameText, return nullptr;);
	virtual UBorder* GetHighlightBorder() const PURE_VIRTUAL(USystemBodyMarkerWidget::GetHighlightBorder, return nullptr;);// Bindings expected to exist (optional)

	virtual UBorder* GetMarkerHighlightBorder() const PURE_VIRTUAL(
		USystemMarkerWidget::GetMarkerHighlightBorder, return nullptr;
	);
	// Material used for the marker image (must be assigned per derived widget instance)
	UPROPERTY(EditAnywhere, Category = "Marker")
	UMaterialInterface* BodyWidgetMaterial = nullptr;

	// Cached state
	UPROPERTY()
	FString CachedName;

	UPROPERTY()
	float CachedRadius = 0.f;

private:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// Derived must broadcast its own delegate (planet vs moon)
	virtual void BroadcastClicked() PURE_VIRTUAL(USystemBodyMarkerWidget::BroadcastClicked, );

	// Derived supplies size-from-radius rule (planet vs moon utils differ)
	virtual float ComputeSizePx(float Radius) const PURE_VIRTUAL(USystemBodyMarkerWidget::ComputeSizePx, return 64.f;);

	// Applies RT into image/material
	void ApplyRT(UTextureRenderTarget2D* InRT);
};

