// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "SystemMapUtils.h"

float SystemMapUtils::ClampZoomLevel(float ProposedZoom, float MinZoom, float MaxZoom)
{
	return FMath::Clamp(ProposedZoom, MinZoom, MaxZoom);
}

float SystemMapUtils::GetCenteredCanvasPosition(float ContentWidth, float ViewportWidth)
{
	return (ViewportWidth - ContentWidth) * 0.5f;
}

float SystemMapUtils::GetCenteredScrollOffset(float ContentHeight, float ViewportHeight)
{
	const float MaxScrollY = FMath::Max(ContentHeight - ViewportHeight, 0.f);
	return MaxScrollY * 0.5f;
}

float SystemMapUtils::InterpolateScrollOffset(float CurrentOffset, float TargetOffset, float DeltaTime, float Speed)
{
	return FMath::FInterpTo(CurrentOffset, TargetOffset, DeltaTime, Speed);
}

float SystemMapUtils::SnapZoomToSteps(float Zoom, float StepSize, float MinZoom, float MaxZoom)
{
	float Clamped = FMath::Clamp(Zoom, MinZoom, MaxZoom);
	float Steps = FMath::RoundToFloat(Clamped / StepSize);
	return FMath::Clamp(Steps * StepSize, MinZoom, MaxZoom);
}

float SystemMapUtils::ClampHorizontalPosition(float ProposedX, float ContentWidth, float ViewportWidth, float Margin)
{
	const float MinVisible = 32.f; // ensure part of the central sun stays on-screen

	// Prevent the left edge from moving too far right (star disappearing)
	const float MaxX = Margin;

	// Prevent the right edge from dragging too far left (entire canvas offscreen)
	const float MinX = FMath::Min(ViewportWidth - ContentWidth + Margin, Margin - MinVisible);
	//const float MinX = ViewportWidth - ContentWidth - Margin;

	return FMath::Clamp(ProposedX, MinX, MaxX);
}

float SystemMapUtils::ClampVerticalPosition(float ProposedY, float ContentHeight, float ViewportHeight, float Margin)
{
	const float MinY = Margin; // Prevents top from going offscreen
	const float MaxY = FMath::Max(ContentHeight - ViewportHeight - Margin, 0.f); // Prevents bottom overscroll

	return FMath::Clamp(ProposedY, MinY, MaxY);
}

float SystemMapUtils::ClampVerticalScroll(float ProposedOffset, float ContentHeight, float ViewportHeight, float Margin)
{
	const float MaxScrollY = FMath::Max(ContentHeight - ViewportHeight - Margin, 0.f);
	const float MinScrollY = Margin;

	return FMath::Clamp(ProposedOffset, MinScrollY, MaxScrollY);
}

float SystemMapUtils::ClampHorizontalScroll(float ProposedOffset, float ContentWidth, float ViewportWidth, float Margin)
{
	const float MaxScrollX = FMath::Max(ContentWidth - ViewportWidth - Margin, 0.f);
	const float MinScrollX = Margin;

	return FMath::Clamp(ProposedOffset, MinScrollX, MaxScrollX);
}

FPlanetFocusResult SystemMapUtils::CenterOnPlanet(
	const FVector2D& MarkerPosition,
	const FVector2D& MarkerSize,
	const FVector2D& ViewportSize,
	const FVector2D& ContentSize,
	float CurrentZoom,
	float RequestedZoom,
	float Margin)
{
	FPlanetFocusResult Result;

	const FVector2D MarkerCenter = MarkerPosition + MarkerSize * 0.5f;

	UE_LOG(LogTemp, Log, TEXT("CenterOnPlanet:"));
	UE_LOG(LogTemp, Log, TEXT("  MarkerPos: %s, MarkerSize: %s, MarkerCenter: %s"), *MarkerPosition.ToString(), *MarkerSize.ToString(), *MarkerCenter.ToString());
	UE_LOG(LogTemp, Log, TEXT("  ViewportSize: %s, ContentSize: %s, CurrentZoom: %.2f, RequestedZoom: %.2f"), *ViewportSize.ToString(), *ContentSize.ToString(), CurrentZoom, RequestedZoom);

	// Scroll Y clamp
	const float ProposedScrollY = MarkerCenter.Y - ViewportSize.Y * 0.5f;
	const float MaxScrollY = FMath::Max(ContentSize.Y - ViewportSize.Y, 0.f);
	const float MinScrollY = 0.f;
	Result.ScrollOffsetY = FMath::Clamp(ProposedScrollY, MinScrollY, MaxScrollY);

	// Canvas X clamp
	const float ProposedCanvasX = ViewportSize.X * 0.5f - MarkerCenter.X;
	const float MaxCanvasX = Margin;
	const float MinCanvasX = ViewportSize.X - ContentSize.X;
	Result.CanvasOffsetX = FMath::Clamp(ProposedCanvasX, MinCanvasX, MaxCanvasX);

	UE_LOG(LogTemp, Log, TEXT("  ProposedCanvasX: %.1f, ClampedCanvasX: %.1f (MinX: %.1f, MaxX: %.1f)"), ProposedCanvasX, Result.CanvasOffsetX, MinCanvasX, MaxCanvasX);

	// Zoom level
	Result.ZoomLevel = (RequestedZoom > 0.f)
		? ClampZoomLevel(RequestedZoom)
		: CurrentZoom;

	UE_LOG(LogTemp, Log, TEXT("  Final ZoomLevel: %.2f"), Result.ZoomLevel);

	return Result;
}