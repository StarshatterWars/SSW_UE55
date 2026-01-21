/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright © 2025-2026. All Rights Reserved.

	ORIGINAL AUTHOR AND STUDIO: John DiCamillo / Destroyer Studios LLC

	SUBSYSTEM:    Stars.exe
	FILE:         Terrain.cpp
	AUTHOR:       Carlos Bott


	OVERVIEW
	========
*/

#include "Terrain.h"
#include "TerrainApron.h"
#include "TerrainClouds.h"
#include "TerrainLayer.h"
#include "TerrainPatch.h"
#include "TerrainRegion.h"
#include "Water.h"

#include "CameraView.h"
#include "SimProjector.h"
#include "StarSystem.h"
#include "SimScene.h"
#include "Bitmap.h"
#include "DataLoader.h"
#include "Game.h"

#include "HAL/UnrealMemory.h"        // FMemory::Memzero
#include "Math/UnrealMathUtility.h"  // FMath
#include <cmath>                     // floor

// +--------------------------------------------------------------------+

int Terrain::detail_level = 3; // default = MEDIUM DETAIL

static const int PATCH_SIZE = 16;

// +--------------------------------------------------------------------+

Terrain::Terrain(TerrainRegion* trgn)
	: region(trgn)
	, patches(nullptr)
	, water_patches(nullptr)
	, water(nullptr)
	, aprons(nullptr)
	, clouds(nullptr)
	, terrain_normals(nullptr)
{
	detail_frame = 0;
	datapath = "Galaxy/";
	terrain_texture = nullptr;
	apron_texture = nullptr;
	water_texture = nullptr;

	for (int i = 0; i < 6; i++)
		env_texture[i] = nullptr;

	for (int i = 0; i < 256; i++)
		tiles[i] = nullptr;

	for (int i = 0; i < 2; i++) {
		cloud_texture[i] = nullptr;
		shade_texture[i] = nullptr;
		noise_texture[i] = nullptr;
	}

	if (region) {
		scale = region->LateralScale();
		mtnscale = region->MountainScale();

		ListIter<TerrainLayer> iter = region->GetLayers();
		while (++iter) {
			TerrainLayer* orig = iter.value();
			TerrainLayer* copy = new TerrainLayer;
			*copy = *orig;
			layers.append(copy);
		}

		layers.sort();
	}
	else {
		scale = 1;
		mtnscale = 1;
	}
}

// +--------------------------------------------------------------------+

Terrain::~Terrain()
{
	if (patches) {
		for (int i = 0; i < subdivisions; i++) {
			for (int j = 0; j < subdivisions; j++) {
				GRAPHIC_DESTROY(patches[i * subdivisions + j]);
			}
		}
	}

	if (water_patches) {
		for (int i = 0; i < subdivisions; i++) {
			for (int j = 0; j < subdivisions; j++) {
				GRAPHIC_DESTROY(water_patches[i * subdivisions + j]);
			}
		}
	}

	if (aprons) {
		for (int i = 0; i < 8; i++) {
			GRAPHIC_DESTROY(aprons[i]);
		}
	}

	if (clouds) {
		for (int i = 0; i < nclouds; i++) {
			GRAPHIC_DESTROY(clouds[i]);
		}
	}

	if (water) {
		for (int i = 0; i < 6; i++) {
			delete water[i];
		}
	}

	delete[] aprons;
	delete[] clouds;
	delete[] patches;
	delete[] water_patches;
	delete[] water;
	delete[] terrain_normals;

	terrain_patch.ClearImage();
	terrain_apron.ClearImage();

	layers.destroy();
}

// +--------------------------------------------------------------------+

void
Terrain::BuildTerrain()
{
	DataLoader* loader = DataLoader::GetLoader();
	loader->SetDataPath(FString(datapath.data())); 

	loader->LoadBitmap(region->PatchName(), terrain_patch);
	loader->LoadBitmap(region->ApronName(), terrain_apron);

	loader->LoadTexture(region->PatchTexture(), terrain_texture);
	loader->LoadTexture(region->ApronTexture(), apron_texture);

	if (region->WaterTexture().length()) {
		loader->LoadTexture(region->WaterTexture(), water_texture);

		if (region->EnvironmentTexture(0).length() > 0) {
			loader->LoadTexture(region->EnvironmentTexture(0), env_texture[0]);
			loader->LoadTexture(region->EnvironmentTexture(1), env_texture[1]);
			loader->LoadTexture(region->EnvironmentTexture(2), env_texture[2]);
			loader->LoadTexture(region->EnvironmentTexture(3), env_texture[3]);
			loader->LoadTexture(region->EnvironmentTexture(4), env_texture[4]);
			loader->LoadTexture(region->EnvironmentTexture(5), env_texture[5]);
		}
	}

	loader->LoadTexture(region->CloudsHigh(), cloud_texture[0], Bitmap::BMP_TRANSLUCENT);
	loader->LoadTexture(region->CloudsLow(), cloud_texture[1], Bitmap::BMP_TRANSLUCENT);
	loader->LoadTexture(region->ShadesLow(), shade_texture[1], Bitmap::BMP_TRANSLUCENT);

	if (region->DetailTexture0().length())
		loader->LoadTexture(region->DetailTexture0(), noise_texture[0], Bitmap::BMP_TRANSLUCENT, false, true);

	if (region->DetailTexture1().length())
		loader->LoadTexture(region->DetailTexture1(), noise_texture[1], Bitmap::BMP_TRANSLUCENT, false, true);

	subdivisions = terrain_patch.Width() / PATCH_SIZE;
	patch_size = terrain_patch.Width() / subdivisions;

	BuildNormals();

	const double dx = scale * patch_size;
	const double dz = scale * patch_size;
	const double offset = -subdivisions / 2.0;

	if (water_texture) {
		water = new Water * [6];
		for (int i = 0; i < 6; i++) {
			water[i] = new Water();
			const int n = (1 << i) + 1;
			water[i]->Init(n, (float)(scale * patch_size), 90.0f);
		}
	}

	// load tile textures:
	for (int i = 0; i < layers.size(); i++) {
		TerrainLayer* layer = layers.at(i);

		if (i < layers.size() - 1)
			layer->max_height = layers.at(i + 1)->min_height;
		else
			layer->max_height = 1e6;

		if (layer->tile_name.length())
			loader->LoadTexture(layer->tile_name, layer->tile_texture);

		if (layer->detail_name.length())
			loader->LoadTexture(layer->detail_name, layer->detail_texture);
	}

	patches = new TerrainPatch * [subdivisions * subdivisions];

	if (water_texture)
		water_patches = new TerrainPatch * [subdivisions * subdivisions];

	for (int i = 0; i < subdivisions; i++) {
		for (int j = 0; j < subdivisions; j++) {
			const int j1 = subdivisions - j;

			Rect   rect(j * patch_size, i * patch_size, patch_size, patch_size);
			FVector p1((j1 + offset) * dx, 0.0, (i + offset) * dz);
			FVector p2((j1 + offset + 1) * dx, (double)mtnscale, (i + offset + 1) * dz);

			const int index = i * subdivisions + j;
			patches[index] = new TerrainPatch(this, &terrain_patch, rect, p1, p2);

			if (water_texture && patches[index]->MinHeight() < 3) {
				water_patches[index] = new TerrainPatch(this, rect, p1, p2, 30);
			}
			else if (water_patches) {
				water_patches[index] = nullptr;
			}
		}
	}

	int a = 0;

	const double dx2 = scale * terrain_patch.Width();
	const double dz2 = scale * terrain_patch.Height();
	const double offset2 = -3.0 / 2.0;
	const double xoffset = offset2 + 1.0 / 16.0;

	aprons = new TerrainApron * [8];

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			const int j1 = 2 - j;

			if (i != 1 || j1 != 1) {
				Rect   rect(j * subdivisions, i * subdivisions, subdivisions, subdivisions);
				FVector p1((j1 + xoffset) * dx2, 0.0, (i + offset2) * dz2);
				FVector p2((j1 + xoffset + 1) * dx2, (double)mtnscale, (i + offset2 + 1) * dz2);

				aprons[a++] = new TerrainApron(this, &terrain_apron, rect, p1, p2);
			}
		}
	}

	Weather::STATE state = region->GetWeather().State();

	if (state == Weather::HIGH_CLOUDS || state == Weather::MODERATE_CLOUDS) {
		double altitude = region->CloudAltHigh();
		nclouds = 9;

		if (state == Weather::MODERATE_CLOUDS)
			nclouds *= 2;

		clouds = new TerrainClouds * [nclouds];

		a = 0;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				clouds[a] = new TerrainClouds(this, 0);

				const double xloc = (j - 1) * 75000.0 + (FMath::FRand() - 0.5) * 50000.0;
				const double yloc = (i - 1) * 75000.0 + (FMath::FRand() - 0.5) * 50000.0;

				clouds[a]->MoveTo(FVector(xloc, altitude, yloc));
				a++;
			}
		}

		if (state == Weather::MODERATE_CLOUDS) {
			altitude = region->CloudAltLow();

			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					clouds[a] = new TerrainClouds(this, 1);

					const double xloc = (j - 1) * 75000.0 + (FMath::FRand() - 0.5) * 50000.0;
					const double yloc = (i - 1) * 75000.0 + (FMath::FRand() - 0.5) * 50000.0;

					clouds[a]->MoveTo(FVector(xloc, altitude, yloc));
					a++;
				}
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
Terrain::BuildNormals()
{
	if (terrain_normals) {
		delete[] terrain_normals;
		terrain_normals = nullptr;
	}

	const int w = terrain_patch.Width();
	const int h = terrain_patch.Height();

	FColor* pix = terrain_patch.HiPixels();

	BYTE* alt = new BYTE[h * w];
	const int nverts = w * h;

	const double nscale = region->MountainScale() / (region->LateralScale() * 2.0);

	terrain_normals = new Vec3B[nverts];
	FMemory::Memzero(terrain_normals, sizeof(Vec3B) * nverts);

	for (int i = 0; i < w; i++) {
		alt[(0) * w + i] = 0;
		alt[(h - 1) * w + i] = 0;

		terrain_normals[(0) * w + i] = Vec3B(128, 128, 255);
		terrain_normals[(h - 1) * w + i] = Vec3B(128, 128, 255);
	}

	for (int i = 0; i < h; i++) {
		alt[i * w + 0] = 0;
		alt[i * w + (w - 1)] = 0;

		terrain_normals[i * w + 0] = Vec3B(128, 128, 255);
		terrain_normals[i * w + (w - 1)] = Vec3B(128, 128, 255);
	}

	for (int y = 1; y < h - 1; y++) {
		for (int x = 1; x < w - 1; x++) {
			alt[y * w + x] = (BYTE)pix[y * w + x].R;
		}
	}

	for (int y = 1; y < h - 1; y++) {
		for (int x = 1; x < w - 1; x++) {
			const double dx =
				(alt[y * w + (x - 1)] - alt[y * w + (x + 1)]) * nscale +
				(alt[(y - 1) * w + (x - 1)] - alt[(y - 1) * w + (x + 1)]) * nscale * 0.5 +
				(alt[(y + 1) * w + (x - 1)] - alt[(y + 1) * w + (x + 1)]) * nscale * 0.5;

			const double dy =
				(alt[(y - 1) * w + x] - alt[(y + 1) * w + x]) * nscale +
				(alt[(y - 1) * w + (x - 1)] - alt[(y + 1) * w + (x - 1)]) * nscale * 0.5 +
				(alt[(y - 1) * w + (x + 1)] - alt[(y + 1) * w + (x + 1)]) * nscale * 0.5;

			FVector norm(dx, dy, 1.0);
			norm.Normalize();

			Vec3B* tnorm = &terrain_normals[y * w + x];

			tnorm->x = (BYTE)(norm.X * 127.0 + 128.0);
			tnorm->y = (BYTE)(norm.Y * 127.0 + 128.0);
			tnorm->z = (BYTE)(norm.Z * 127.0 + 128.0);
		}
	}

	delete[] alt;
}

// +--------------------------------------------------------------------+

void
Terrain::Activate(SimScene& scene)
{
	StarSystem* system = region->System();
	if (system)
		datapath = system->GetDataPath();

	region->GetWeather().Update();

	if (!patches)
		BuildTerrain();

	if (patches) {
		for (int i = 0; i < subdivisions; i++) {
			for (int j = 0; j < subdivisions; j++) {
				if (patches[i * subdivisions + j])
					scene.AddGraphic(patches[i * subdivisions + j]);
			}
		}
	}

	if (water_patches) {
		for (int i = 0; i < subdivisions; i++) {
			for (int j = 0; j < subdivisions; j++) {
				if (water_patches[i * subdivisions + j])
					scene.AddGraphic(water_patches[i * subdivisions + j]);
			}
		}
	}

	if (aprons) {
		for (int i = 0; i < 8; i++) {
			if (aprons[i])
				scene.AddGraphic(aprons[i]);
		}
	}

	if (clouds) {
		for (int i = 0; i < nclouds; i++) {
			if (clouds[i])
				scene.AddGraphic(clouds[i]);
		}
	}
}

void
Terrain::Deactivate(SimScene& scene)
{
	if (patches) {
		for (int i = 0; i < subdivisions; i++) {
			for (int j = 0; j < subdivisions; j++) {
				TerrainPatch* p = patches[i * subdivisions + j];
				if (p) {
					p->DeletePrivateData();
					scene.DelGraphic(p);
				}
			}
		}
	}

	if (water_patches) {
		for (int i = 0; i < subdivisions; i++) {
			for (int j = 0; j < subdivisions; j++) {
				TerrainPatch* p = water_patches[i * subdivisions + j];
				if (p) {
					p->DeletePrivateData();
					scene.DelGraphic(p);
				}
			}
		}
	}

	if (aprons) {
		for (int i = 0; i < 8; i++) {
			if (aprons[i])
				scene.DelGraphic(aprons[i]);
		}
	}

	if (clouds) {
		for (int i = 0; i < nclouds; i++) {
			if (clouds[i])
				scene.DelGraphic(clouds[i]);
		}
	}

	StarSystem* system = region->System();

	// restore sunlight color and brightness on exit:
	if (system) {
		system->RestoreTrueSunColor();
	}
}

// +--------------------------------------------------------------------+

void
Terrain::ExecFrame(double seconds)
{
	if (water) {
		for (int i = 0; i < 6; i++) {
			if (water[i])
				water[i]->CalcWaves(seconds);
		}
	}
}

// +--------------------------------------------------------------------+

void
Terrain::SelectDetail(SimProjector* projector)
{
	if (!patches)
		return;

	if (detail_frame >= Game::Frame())
		return;

	// compute detail map:
	for (int z = 0; z < subdivisions; z++) {
		for (int x = 0; x < subdivisions; x++) {
			TerrainPatch* patch = patches[z * subdivisions + x];
			int          ndetail = 0;
			FVector      loc = patch->Location();
			float        radius = patch->Radius();

			if (loc.Length() < 2.0 * radius) {
				ndetail = detail_level;
			}
			else {
				const double threshold = 4.0;

				for (int level = 1; level <= detail_level; level++) {
					const double feature_size = radius / (1 << level);

					if (projector->ApparentRadius(loc, (float)feature_size) > threshold)
						ndetail = level;
				}
			}

			patch->SetDetailLevel(ndetail);

			if (water_patches) {
				TerrainPatch* wpatch = water_patches[z * subdivisions + x];
				if (wpatch)
					wpatch->SetDetailLevel(ndetail);
			}
		}
	}

	// compute fog fade level:
	const double hour = region->DayPhase();

	if (hour < 12)
		fog_fade = (hour) / 12.0;
	else
		fog_fade = (24.0 - hour) / 12.0;

	fog_fade = fog_fade * (1.0 - region->HazeFade()) + region->HazeFade();

	detail_frame = Game::Frame();
}

// +--------------------------------------------------------------------+

double
Terrain::Height(double x, double y) const
{
	double h = 0;

	if (patches) {
		int ix = (int)std::floor(x / (patch_size * scale));
		int iy = (int)std::floor(y / (patch_size * scale));

		const double px = x - ix * patch_size * scale;
		const double py = y - iy * patch_size * scale;

		ix = subdivisions / 2 - ix;
		iy = subdivisions / 2 + iy;

		TerrainPatch* patch = nullptr;

		if (ix >= 0 && ix < subdivisions &&
			iy >= 0 && iy < subdivisions) {
			patch = patches[iy * subdivisions + ix];
		}

		if (patch)
			h = patch->Height(px, py);
	}

	if (water_patches && h < 30)
		h = 30;

	return h;
}

// +--------------------------------------------------------------------+

void
Terrain::SetDetailLevel(int detail)
{
	if (detail >= 1 && detail <= 4) {
		detail_level = detail;
	}
}

// +--------------------------------------------------------------------+

bool
Terrain::IsFirstPatch(TerrainPatch* p) const
{
	return (patches && *patches == p);
}
