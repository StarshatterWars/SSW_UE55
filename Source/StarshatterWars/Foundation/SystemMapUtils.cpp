// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "SystemMapUtils.h"

float SystemMapUtils::ClampVerticalScroll(float ProposedOffset, float ContentHeight, float ViewportHeight, float Margin)
{
	const float MaxScrollY = FMath::Max(ContentHeight - ViewportHeight - Margin, 0.f);
	const float MinScrollY = Margin;

	return FMath::Clamp(ProposedOffset, MinScrollY, MaxScrollY);
}

float SystemMapUtils::ClampHorizontalPosition(float ProposedX, float ContentWidth, float ViewportWidth, float Margin)
{
	const float MinX = FMath::Min(ViewportWidth - ContentWidth + Margin, Margin);
	const float MaxX = Margin;

	return FMath::Clamp(ProposedX, MinX, MaxX);
}


