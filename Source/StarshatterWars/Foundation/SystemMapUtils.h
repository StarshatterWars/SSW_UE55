// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */

struct STARSHATTERWARS_API FPlanetFocusResult
{
	float ScrollOffsetY = 0.f;
	float CanvasOffsetX = 0.f;
	float ZoomLevel = -1.f;
};

class STARSHATTERWARS_API SystemMapUtils
{

public:	
	
	UFUNCTION()
	static float ClampZoomLevel(float ProposedZoom, float MinZoom = 0.5f, float MaxZoom = 3.0f);

	UFUNCTION()
	static float SnapZoomToSteps(float Zoom, float StepSize = 0.1f, float MinZoom = 0.5f, float MaxZoom = 3.0f);

	UFUNCTION()
	static float GetCenteredCanvasPosition(float ContentWidth, float ViewportWidth);

	UFUNCTION()
	static float GetCenteredScrollOffset(float ContentHeight, float ViewportHeight);

	UFUNCTION()
	static float InterpolateScrollOffset(float CurrentOffset, float TargetOffset, float DeltaTime, float Speed = 10.f);

	UFUNCTION()
	static float ClampVerticalPosition(float ProposedY, float ContentHeight, float ViewportHeight, float Margin = 50.f);

	UFUNCTION()
	static float ClampHorizontalPosition(float ProposedY, float ContentHeight, float ViewportHeight, float Margin = 50.f);

	UFUNCTION()
	static float ClampVerticalScroll(float ProposedOffset, float ContentHeight, float ViewportHeight, float Margin = 50.f);

	UFUNCTION()
	static float ClampHorizontalScroll(float ProposedOffset, float ContentWidth, float ViewportWidth, float Margin = 50.f);

	// Cubic smoothstep: ease-in/out
	static float EaseInOut(float t);

	UFUNCTION()
	static FPlanetFocusResult CenterOnPlanet(
		const FVector2D& MarkerPosition,
		const FVector2D& MarkerSize,
		const FVector2D& ViewportSize,
		const FVector2D& ContentSize,
		float CurrentZoom,
		float RequestedZoom = -1.f,
		float Margin = 50.f);

	// Returns a clamped canvas offset that keeps content partially visible within the viewport
	UFUNCTION()
	static FVector2D ClampCanvasDragOffset(
		const FVector2D& ProposedOffset,
		const FVector2D& ContentSize,
		const FVector2D& ViewportSize,
		float Padding = 50.0f);
};
