// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/GameStructs.h" // FS_Galaxy struct
#include "../System/SSWGameInstance.h"
#include "SystemOrbitWidget.generated.h"

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API USystemOrbitWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetOrbitRadius(float InRadius) { OrbitRadius = InRadius; Invalidate(EInvalidateWidget::Paint); }
	void SetOrbitTilt(float InTilt) { OrbitTiltY = InTilt;     Invalidate(EInvalidateWidget::Paint); }
	
protected:
	virtual int32 NativePaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;

private:
	float OrbitRadius = 100.0f;  // distance from center in screen units
	float OrbitTiltY = 1.0f;    // scale Y-axis for elliptical tilt (0.6 = flatter)
};