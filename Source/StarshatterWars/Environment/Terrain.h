/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright © 2025-2026. All Rights Reserved.

	ORIGINAL AUTHOR AND STUDIO: John DiCamillo / Destroyer Studios LLC

	SUBSYSTEM:    Stars.exe
	FILE:         Terrain.h
	AUTHOR:       Carlos Bott


	OVERVIEW
	========
	Test pseudo-random terrain heightfield, based on Solid
*/

#pragma once

#include "Types.h"
#include "Graphic.h"
#include "Geometry.h"
#include "Text.h"
#include "Bitmap.h"

#include "Math/Vector.h"   // FVector

// Render assets migrated to UE types:
class UTexture2D;

// +--------------------------------------------------------------------+

class SimProjector;
class SimScene;
class TerrainApron;
class TerrainClouds;
class TerrainLayer;
class TerrainPatch;
class TerrainRegion;
class Water;

// +--------------------------------------------------------------------+

struct Vec3B
{
	Vec3B() : x(0), y(0), z(0) {}
	Vec3B(BYTE a, BYTE b, BYTE c) : x(a), y(b), z(c) {}

	BYTE x, y, z;
};

// +--------------------------------------------------------------------+

class Terrain
{
public:
	explicit Terrain(TerrainRegion* InRegion);
	virtual ~Terrain();

	virtual void      Activate(SimScene& scene);
	virtual void      Deactivate(SimScene& scene);

	virtual void      SelectDetail(SimProjector* proj);
	virtual void      BuildTerrain();
	virtual void      BuildNormals();

	virtual void      ExecFrame(double seconds);

	// NOTE:
	// Original signature was Height(double x, double y) but the caller usage
	// in Starshatter commonly treats the second parameter as "z".
	// Kept as-is to avoid ripple changes elsewhere.
	double            Height(double x, double y) const;

	const Vec3B* Normals()      const { return terrain_normals; }
	TerrainRegion* GetRegion() { return region; }
	double            FogFade()      const { return fog_fade; }

	// Textures (converted from Bitmap*):
	UTexture2D* Texture() { return terrain_texture; }
	UTexture2D* ApronTexture() { return apron_texture; }
	UTexture2D* WaterTexture() { return water_texture; }
	UTexture2D** EnvironmentTexture() { return env_texture; }
	UTexture2D* TileTexture(int n) { return tiles[n]; }
	UTexture2D* CloudTexture(int n) { return cloud_texture[n]; }
	UTexture2D* ShadeTexture(int n) { return shade_texture[n]; }
	UTexture2D* DetailTexture(int n) { return noise_texture[n]; }

	Water* GetWater(int level) { return water[level]; }
	List<TerrainLayer>& GetLayers() { return layers; }

	bool              IsFirstPatch(TerrainPatch* p) const;

	static int        DetailLevel() { return detail_level; }
	static void       SetDetailLevel(int detail);

protected:
	TerrainRegion* region = nullptr;
	TerrainPatch** patches = nullptr;
	TerrainPatch** water_patches = nullptr;
	Water** water = nullptr;
	TerrainApron** aprons = nullptr;
	TerrainClouds** clouds = nullptr;
	int               nclouds = 0;

	// Patch/apron source images were Bitmaps; keep the data containers if they
	// still exist in your Starshatter core (used by software terrain building).
	// If you fully migrate the build pipeline to UE, these may be removed later.
	Bitmap            terrain_patch;
	Bitmap            terrain_apron;

	// Runtime textures migrated to UE:
	UTexture2D* terrain_texture = nullptr;
	UTexture2D* apron_texture = nullptr;
	UTexture2D* water_texture = nullptr;
	UTexture2D* env_texture[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	UTexture2D* tiles[256] = { nullptr };
	UTexture2D* cloud_texture[2] = { nullptr, nullptr };
	UTexture2D* shade_texture[2] = { nullptr, nullptr };
	UTexture2D* noise_texture[2] = { nullptr, nullptr };

	Vec3B* terrain_normals = nullptr;
	List<TerrainLayer> layers;

	Text              datapath;
	double            scale = 0.0;
	double            mtnscale = 0.0;
	int               subdivisions = 0;
	int               patch_size = 0;
	DWORD             detail_frame = 0;
	double            fog_fade = 0.0;

	static int        detail_level;
};
