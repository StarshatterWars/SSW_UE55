/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright © 2025-2026. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         TerrainHaze.cpp
	AUTHOR:       Carlos Bott

	ORIGINAL AUTHOR AND STUDIO:
	John DiCamillo, Destroyer Studios LLC

	OVERVIEW
	========
*/

#include "TerrainHaze.h"
#include "Terrain.h"
#include "TerrainRegion.h"

#include "CoreMinimal.h"

// +====================================================================+
// NOTE:
// The original implementation used a file-static Bitmap. In UE, render
// assets are typically UTexture2D* managed elsewhere. This unit does not
// require a texture reference, so it has been removed.
// +====================================================================+

TerrainHaze::TerrainHaze()
	: tregion(nullptr)
{
}

TerrainHaze::~TerrainHaze()
{
}

void
TerrainHaze::Render(Video* video, DWORD flags)
{
	if (flags & RENDER_ADDITIVE)
		return;

	if (!model)
		return;

	if (!Luminous()) {
		SetLuminous(true);
		model->SetDynamic(true);
	}

	Surface* surface = model->GetSurfaces().first();
	if (!surface)
		return;

	uint32 Sky = 0;
	uint32 Fog = 0;

	if (tregion) {
		Sky = tregion->SkyColor().Value();
		Fog = tregion->FogColor().Value();
	}

	// Clear the solid lights to ambient:
	VertexSet* vset = surface->GetVertexSet();
	if (!vset)
		return;

	for (int i = 0; i < vset->nverts; i++) {
		// vset->loc is now FVector (via Vec3/Point migration)
		vset->diffuse[i] = (vset->loc[i].Y > 0.0f) ? Sky : Fog;
	}

	InvalidateSurfaceData();
	Solid::Render(video, flags);
}

int
TerrainHaze::CheckRayIntersection(FVector Q, FVector w, double len, FVector& ipt, bool ttpas)
{
	return 0;
}
