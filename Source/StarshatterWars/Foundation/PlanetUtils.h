// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "../Game/GameStructs.h"

/**
 * 
 */

class STARSHATTERWARS_API PlanetUtils
{
public:
	/** Loads Planet Texture */
	static UTexture2D* LoadPlanetTexture(const FString& TextureName);

	/** Gets a UI image scale (in pixels) from radius using log scale */
	static float GetUISizeFromRadius(float Radius, float MinSize = 48.f, float MaxSize = 128.f);

	/** Creates a unique 512x512 RGBA8 render target with black clear color */
	static UTextureRenderTarget2D* CreatePlanetRenderTarget(const FString& BaseName, UObject* Outer = nullptr, int32 Resolution = 256);

	/** Load uasset materials */
	static UTexture2D* LoadPlanetAssetTexture(const FString& TextureName);

	// Estimate ideal render target resolution based on planet radius in kilometers
	static int32 GetRenderTargetResolutionForRadius(double RadiusKm);

	// Scale the UI based on planet radius in kilometers
	static float GetPlanetUIScale(double RadiusKm); // in kilometers

private:
	static TMap<FString, UTexture2D*> LoadedTextureCache;


};
