// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "SelectionLinesWidget.h"
#include "Layout/Geometry.h"             // FGeometry
#include "Layout/PaintGeometry.h"        // ToPaintGeometry
#include "Styling/SlateColor.h"          // FSlateColor / FLinearColor
#include "Rendering/DrawElements.h"

void USelectionLinesWidget::SetMarkerCenter(FVector2D ScreenCenter)
{
    FVector2D MarkerOffset(-48.f, -12.f);
    MarkerCenter = ScreenCenter + MarkerOffset;
    Invalidate(EInvalidateWidget::Paint);
}

int32 USelectionLinesWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
    const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
    int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    const FVector2D Size = AllottedGeometry.GetLocalSize();
    const FLinearColor LineColor = FLinearColor::Yellow;

    // Horizontal (left to right)
    FSlateDrawElement::MakeLines(
        OutDrawElements,
        LayerId,
        AllottedGeometry.ToPaintGeometry(),
        { FVector2D(0.f, MarkerCenter.Y), FVector2D(Size.X, MarkerCenter.Y) },
        ESlateDrawEffect::None,
        LineColor,
        true,
        1.f
    );

    // Vertical (top to bottom)
    FSlateDrawElement::MakeLines(
        OutDrawElements,
        LayerId,
        AllottedGeometry.ToPaintGeometry(),
        { FVector2D(MarkerCenter.X, 0.f), FVector2D(MarkerCenter.X, Size.Y) },
        ESlateDrawEffect::None,
        LineColor,
        true,
        1.f
    );

    return LayerId + 1;
}

