// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "SystemOrbitWidget.h"
#include "Layout/Geometry.h"             // FGeometry
#include "Layout/PaintGeometry.h"        // ToPaintGeometry
#include "Styling/SlateColor.h"          // FSlateColor / FLinearColor
#include "Rendering/DrawElements.h"

/*int32 USystemOrbitWidget::NativePaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	const FLinearColor OrbitColor = FLinearColor::Gray;
	const FVector2D Center = AllottedGeometry.GetLocalSize() * 0.5f;

	TArray<FVector2D> Points;
	const int32 SegmentCount = 64;

	for (int32 i = 0; i <= SegmentCount; ++i)
	{
		const float Angle = FMath::DegreesToRadians(i * 360.0f / SegmentCount);

		// Elliptical orbit with vertical tilt applied
		const float X = FMath::Cos(Angle) * OrbitRadius;
		const float Y = FMath::Sin(Angle) * OrbitRadius * OrbitTiltY;

		Points.Add(Center + FVector2D(X, Y));
	}

	// Draw the orbit path
	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),
		Points,
		ESlateDrawEffect::None,
		OrbitColor,
		true,
		1.0f
	);

	return LayerId + 1;
}*/

int32 USystemOrbitWidget::NativePaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled
) const
{
	const FLinearColor OrbitColor = FLinearColor::Gray;
	const FVector2D Center = AllottedGeometry.GetLocalSize() * 0.5f;

	const int32 SegmentCount = 64;
	TArray<FVector2D> Points;
	Points.Reserve(SegmentCount + 1);

	for (int32 i = 0; i <= SegmentCount; ++i)
	{
		const float AngleRad = FMath::DegreesToRadians(i * 360.0f / SegmentCount);

		const float X = FMath::Cos(AngleRad) * OrbitRadius;
		const float Y = FMath::Sin(AngleRad) * OrbitRadius;
		const float YTilted = Y * OrbitTiltY;

		Points.Add(Center + FVector2D(X, YTilted));
	}

	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),
		Points,
		ESlateDrawEffect::None,
		OrbitColor,
		true,
		1.0f
	);

	return LayerId + 1;
}
