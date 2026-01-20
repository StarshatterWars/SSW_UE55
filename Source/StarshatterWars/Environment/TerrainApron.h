/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright © 2025-2026. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         TerrainApron.h
	AUTHOR:       Carlos Bott

	ORIGINAL AUTHOR AND STUDIO:
	John DiCamillo, Destroyer Studios LLC

	OVERVIEW
	========
	A Single Edge Section of a Terrain Object
*/

#pragma once

#include "Types.h"
#include "Solid.h"

// Minimal Unreal include needed for FVector:
#include "Math/Vector.h"

// +--------------------------------------------------------------------+

class Terrain;
class UTexture2D;
class SimLight;

// +--------------------------------------------------------------------+

class TerrainApron : public Solid
{
public:
	TerrainApron(Terrain* terrain,
		const UTexture2D* patch, const Rect& rect,
		const FVector& p1, const FVector& p2);
	virtual ~TerrainApron();

	virtual void      Render(Video* video, DWORD flags) override;
	virtual void      Update() override;

	virtual int       CollidesWith(Graphic& o) override;
	virtual bool      Luminous()    const override { return false; }
	virtual bool      Translucent() const override { return false; }

	// accessors:
	double            Scale()         const { return scale; }
	double            MountainScale() const { return mtnscale; }
	double            SeaLevel()      const { return base; }
	void              SetScales(double scale, double mtnscale, double base);

	void              Illuminate(Color ambient, List<SimLight>& lights);
	virtual int       CheckRayIntersection(FVector pt, FVector vpn, double len, FVector& ipt,
		bool treat_translucent_polys_as_solid = true);

protected:
	virtual bool      BuildApron();

	Terrain* terrain = nullptr;

	int               nverts = 0;
	int               npolys = 0;
	int               terrain_width = 0;

	Rect              rect;
	float* heights = nullptr;

	double            scale = 1.0;
	double            mtnscale = 1.0;
	double            base = 0.0;
	double            size = 0.0;
};

