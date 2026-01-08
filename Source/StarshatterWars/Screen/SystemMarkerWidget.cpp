#include "SystemMarkerWidget.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Input/Reply.h"
#include "Engine/Engine.h"
#include "../Foundation/SystemMapUtils.h"

void USystemMarkerWidget::InitCommon(const FString& DisplayName, float Radius, UTextureRenderTarget2D* RenderTarget)
{
    SetVisibility(ESlateVisibility::Visible);

    CachedName = DisplayName;
    CachedRadius = Radius;

    SetToolTipText(FText::FromString(CachedName));

    UTextBlock* NameText = GetMarkerNameText();
    if (NameText)
    {
        NameText->SetText(FText::FromString(CachedName));
        NameText->SetColorAndOpacity(FLinearColor::White);
    }

    UImage* Image = GetMarkerImage();

    if (!Image || !BodyWidgetMaterial)
    {
        UE_LOG(LogTemp, Warning, TEXT("InitCommon(%s): missing Image or Material"), *CachedName);
        return;
    }

    if (!RenderTarget)
    {
        UE_LOG(LogTemp, Warning, TEXT("InitCommon(%s): RenderTarget is null"), *CachedName);
        return;
    }

    UE_LOG(LogTemp, Warning,
        TEXT("InitCommon(%s) Image=%s Text=%s Border=%s"),
        *CachedName,
        *GetNameSafe(GetMarkerImage()),
        *GetNameSafe(GetMarkerNameText()),
        *GetNameSafe(GetMarkerHighlightBorder())
    );

    ApplyRT(RenderTarget); // make ApplyRT use GetMarkerImage() too (see below)
}

void USystemMarkerWidget::SetSelected(bool bSelected)
{
    if (UBorder* Border = GetMarkerHighlightBorder())
    {
        Border->SetVisibility(
            bSelected ? ESlateVisibility::Visible : ESlateVisibility::Hidden
        );
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

void USystemMarkerWidget::ApplyRT(UTextureRenderTarget2D* InRT)
{
    UImage* Image = GetMarkerImage();
    if (!InRT || !Image || !BodyWidgetMaterial)
        return;

    const float SizePx = ComputeSizePx(CachedRadius);

    SystemMapUtils::ApplyRenderTargetToImage(
        this,
        Image,
        BodyWidgetMaterial,
        InRT,
        FVector2D(SizePx, SizePx)
    );
}