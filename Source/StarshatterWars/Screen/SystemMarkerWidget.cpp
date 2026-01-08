#include "SystemMarkerWidget.h"

#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "../Foundation/SystemMapUtils.h"

void USystemMarkerWidget::InitCommon(const FString& DisplayName, float Radius)
{
	SetVisibility(ESlateVisibility::Visible);

	CachedName = DisplayName;
	CachedRadius = Radius;
	bIselected = false;

	SetToolTipText(FText::FromString(CachedName));

	if (UTextBlock* NameText = GetMarkerNameText())
	{
		NameText->SetText(FText::FromString(CachedName));
		NameText->SetColorAndOpacity(FLinearColor::White);
	}

	/*UImage* Img = GetMarkerImage();

	UE_LOG(LogTemp, Warning, TEXT("InitCommon(%s): Img=%s Mat=%s RT=%s"),
		*CachedName,
		*GetNameSafe(Img),
		*GetNameSafe(WidgetMaterial),
		*GetNameSafe(RenderTarget));

	// This is the equivalent of your old: if (!PlanetImage || !PlanetActor || !PlanetWidgetMaterial) return;
	if (!Img || !WidgetMaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitCommon(%s): missing Image or WidgetMaterial"), *CachedName);
		return;
	}

	if (!RenderTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitCommon(%s): RenderTarget is null"), *CachedName);
		return;
	}

	ApplyRT(RenderTarget);*/
}

void USystemMarkerWidget::ApplyRT(UTextureRenderTarget2D* InRT)
{
	UImage* Img = GetMarkerImage();
	if (!InRT || !Img || !WidgetMaterial)
	{
		return;
	}

	const float SizePx = ComputeSizePx(CachedRadius);

	SystemMapUtils::ApplyRenderTargetToImage(
		this,
		Img,
		WidgetMaterial,
		InRT,
		FVector2D(SizePx, SizePx)
	);
}

void USystemMarkerWidget::SetSelected(bool bSelected)
{
	if (UBorder* Border = GetMarkerHighlightBorder())
	{
		Border->SetVisibility(bSelected ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

FReply USystemMarkerWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		BroadcastClicked();
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}
