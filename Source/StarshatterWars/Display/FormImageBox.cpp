/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (C) 2025-2026. All Rights Reserved.

    SUBSYSTEM:    Stars.exe (UE)
    FILE:         FormImageBox.h
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    UE port of legacy FormImageBox control.
*/

#include "FormImageBox.h"

#include "Engine/Texture2D.h"
#include "Widgets/Images/SImage.h"

UFormImageBox::UFormImageBox(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    Brush = FSlateBrush();
}

TSharedRef<SWidget> UFormImageBox::RebuildWidget()
{
    Image =
        SNew(SImage)
        .Image(this, &UFormImageBox::GetSlateBrush);

    return Image.ToSharedRef();
}

void UFormImageBox::SynchronizeProperties()
{
    Super::SynchronizeProperties();

    if (Texture)
    {
        Brush.SetResourceObject(Texture);
    }

    // SImage pulls from GetSlateBrush; invalidation will redraw automatically.
}

const FSlateBrush* UFormImageBox::GetSlateBrush() const
{
    return &Brush;
}

void UFormImageBox::SetTexture(UTexture2D* InTexture)
{
    Texture = InTexture;

    if (Texture)
        Brush.SetResourceObject(Texture);
    else
        Brush.SetResourceObject(nullptr);

    if (Image.IsValid())
        Image->Invalidate(EInvalidateWidget::LayoutAndVolatility);
}

void UFormImageBox::SetBrush(const FSlateBrush& InBrush)
{
    Brush = InBrush;

    // Try to keep Texture in sync if the brush has a UTexture2D:
    Texture = Cast<UTexture2D>(Brush.GetResourceObject());

    if (Image.IsValid())
        Image->Invalidate(EInvalidateWidget::LayoutAndVolatility);
}
