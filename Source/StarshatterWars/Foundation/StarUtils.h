// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "../Game/GameStructs.h" 
#include "Engine/TextureRenderTarget2D.h"

/**
 * 
 */
class STARSHATTERWARS_API StarUtils
{

public:
	/** Get default color for a given spectral class */
	static FLinearColor GetColor(ESPECTRAL_CLASS SpectralClass);

	/** Get emissive glow strength based on spectral class */
	static float GetGlowStrength(ESPECTRAL_CLASS SpectralClass);

	/** Optional: Get sunspot intensity */
	static float GetSunspotStrength(ESPECTRAL_CLASS SpectralClass);

	/** Format class as text label (e.g., "G-Class Star") */
	static FString GetClassName(ESPECTRAL_CLASS SpectralClass);
	
	/** Stellar Rotation */
	static float GetRotationSpeed(float Rotation);

	/** Gets a UI image scale (in pixels) from radius using log scale */
	static float GetUISizeFromRadius(float Radius, float MinSize = 48.f, float MaxSize = 128.f);

	static UTextureRenderTarget2D* CreateRenderTarget(const FString& Name, UObject* Outer);
};
