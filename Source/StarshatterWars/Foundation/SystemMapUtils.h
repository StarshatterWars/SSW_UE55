// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class STARSHATTERWARS_API SystemMapUtils
{

public:	
	/** Clamp the vertical scroll offset with top/bottom margin */
	UFUNCTION()
	static float ClampVerticalScroll(float ProposedOffset, float ContentHeight, float ViewportHeight, float Margin = 50.f);

	/** Clamp the horizontal position of the canvas with left/right margin */
	UFUNCTION()
	static float ClampHorizontalPosition(float ProposedX, float ContentWidth, float ViewportWidth, float Margin = 50.f);
};
