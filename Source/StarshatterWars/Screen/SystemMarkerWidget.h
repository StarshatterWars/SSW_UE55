#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/TextureRenderTarget2D.h"
#include "SystemMarkerWidget.generated.h"

class UImage;
class UBorder;
class UTextBlock;
class UMaterialInterface;

UCLASS(Abstract)
class STARSHATTERWARS_API USystemMarkerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Call from derived InitFromXActor after it sets its data struct
	void InitCommon(const FString& DisplayName, float Radius/*, UTextureRenderTarget2D* RenderTarget*/);

	UFUNCTION()
	virtual void SetSelected(bool bSelected);

protected:
	// Shared material field (replace PlanetWidgetMaterial/MoonWidgetMaterial)
	// Set this in the Widget BP Defaults for PlanetMarkerWidget and MoonMarkerWidget.
	UPROPERTY(EditAnywhere, Category = "Marker")
	UMaterialInterface* WidgetMaterial = nullptr;

	UPROPERTY()
	FString CachedName;

	UPROPERTY()
	float CachedRadius = 0.f;

	UPROPERTY()
	bool bIselected = false;

	// Derived provides actual BP-bound widgets
	virtual UImage* GetMarkerImage() const PURE_VIRTUAL(USystemMarkerWidget::GetMarkerImage, return nullptr;);
	virtual UBorder* GetMarkerHighlightBorder() const PURE_VIRTUAL(USystemMarkerWidget::GetMarkerHighlightBorder, return nullptr;);
	virtual UTextBlock* GetMarkerNameText() const PURE_VIRTUAL(USystemMarkerWidget::GetMarkerNameText, return nullptr;);

	// Derived provides sizing rule
	virtual float ComputeSizePx(float Radius) const PURE_VIRTUAL(USystemMarkerWidget::ComputeSizePx, return 64.f;);

	// Derived provides click broadcast
	virtual void BroadcastClicked() PURE_VIRTUAL(USystemMarkerWidget::BroadcastClicked, );

private:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	void ApplyRT(UTextureRenderTarget2D* InRT);
};
