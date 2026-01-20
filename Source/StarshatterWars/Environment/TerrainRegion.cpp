/*
	Project Starshatter Wars
	Fractal Dev Studios
	Copyright © 2025-2026. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         TerrainRegion.cpp
	AUTHOR:       Carlos Bott

	ORIGINAL AUTHOR AND STUDIO:
	John DiCamillo, Destroyer Studios LLC

	OVERVIEW
	========
	Various Heavenly Bodies
*/

#include "TerrainRegion.h"
#include "Sky.h"
#include "TerrainHaze.h"
#include "TerrainLayer.h"
#include "Sim.h"
#include "Grid.h"
#include "CameraManager.h"
#include "HUDView.h"

#include "Game.h"
#include "Sound.h"
#include "Solid.h"
#include "Sprite.h"
#include "SimLight.h"
#include "Bitmap.h"
#include "DataLoader.h"
#include "SimScene.h"
#include "ParseUtil.h"

// UE:
#include "Math/Vector.h"

// +====================================================================+

TerrainRegion::TerrainRegion(StarSystem* s, const char* n, double r, Orbital* p)
	: OrbitalRegion(s, n, 0.0, r, 0.0, p),
	scale(10e3),
	mtnscale(1e3),
	fog_density(0),
	fog_scale(0),
	day_phase(0),
	eclipsed(false)
{
	type = TERRAIN;

	if (primary) {
		orbit = primary->Radius();
		period = primary->Rotation();
	}

	clouds_alt_high = 12.0e3;
	clouds_alt_low = 8.0e3;

	Update();
}

// +--------------------------------------------------------------------+

TerrainRegion::~TerrainRegion()
{
	layers.destroy();
}

// +--------------------------------------------------------------------+

void
TerrainRegion::Update()
{
	if (!system || !primary)
		return;

	// -----------------------------
	// Orbital update (UE math)
	// -----------------------------
	phase = primary->RotationPhase();

	loc = primary->Location() + FVector(
		orbit * FMath::Cos(phase),
		orbit * FMath::Sin(phase),
		0.0
	);

	if (rep)
		rep->MoveTo(loc);

	// -----------------------------
	// Calc day phase
	// -----------------------------
	OrbitalBody* star = nullptr;
	if (system->Bodies().size() > 0)
		star = system->Bodies()[0];

	if (!star)
		return;

	FVector tvpn = Location() - primary->Location();
	FVector tvst = star->Location() - primary->Location();

	// SafeNormalize avoids NaNs if length is ~0
	tvpn = tvpn.GetSafeNormal();
	tvst = tvst.GetSafeNormal();

	// dot product (clamped for acos domain safety)
	const double dot = FMath::Clamp(static_cast<double>(FVector::DotProduct(tvpn, tvst)), -1.0, 1.0);

	day_phase = FMath::Acos(dot) + PI;

	const FVector meridian = FVector::CrossProduct(tvpn, tvst);

	if (meridian.Z < 0.0) {
		day_phase = 2.0 * PI - day_phase;
	}

	day_phase *= 24.0 / (2.0 * PI);

	if (system->ActiveRegion() != this)
		return;

	// -----------------------------
	// Set sky color
	// -----------------------------
	int base_hour = static_cast<int>(day_phase);

	// Safety clamp so base_hour+1 stays in range [0..24]
	if (base_hour < 0)  base_hour = 0;
	if (base_hour > 23) base_hour = 23;

	const double fraction = day_phase - base_hour;

	sky_color[24] = Color::Scale(sky_color[base_hour], sky_color[base_hour + 1], fraction);
	sun_color[24] = Color::Scale(sun_color[base_hour], sun_color[base_hour + 1], fraction);
	fog_color[24] = Color::Scale(fog_color[base_hour], fog_color[base_hour + 1], fraction);
	ambient[24] = Color::Scale(ambient[base_hour], ambient[base_hour + 1], fraction);
	overcast[24] = Color::Scale(overcast[base_hour], overcast[base_hour + 1], fraction);
	cloud_color[24] = Color::Scale(cloud_color[base_hour], cloud_color[base_hour + 1], fraction);
	shade_color[24] = Color::Scale(shade_color[base_hour], shade_color[base_hour + 1], fraction);

	CameraManager* cam_dir = CameraManager::GetInstance();

	double alt = 0.0;
	double dim = 1.0;

	if (cam_dir && cam_dir->GetCamera())
		alt = cam_dir->GetCamera()->Pos().Y; // UE: FVector uses X/Y/Z

	if (alt > 0.0) {
		if (alt < TERRAIN_ALTITUDE_LIMIT) {
			if (weather.Ceiling() > 0.0) {
				fog_color[24] = overcast[24];
				sky_color[24] = overcast[24];
				dim = 0.125;
			}
			else {
				sky_color[24] = sky_color[24] * (1.0 - alt / TERRAIN_ALTITUDE_LIMIT);
			}
		}
		else {
			sky_color[24] = Color::Black;
		}
	}

	system->SetSunlight(sun_color[24], dim);
	system->SetBacklight(sky_color[24], dim);

	HUDView* hud = HUDView::GetInstance();
	if (hud) {
		Color night_vision = hud->Ambient();
		sky_color[24] += night_vision * 0.15;
	}
}

// +--------------------------------------------------------------------+

void
TerrainRegion::LoadSkyColors(const char* bmp_name)
{
	Bitmap sky_colors_bmp;

	DataLoader* loader = DataLoader::GetLoader();
	loader->LoadBitmap(bmp_name, sky_colors_bmp);

	int max_color = sky_colors_bmp.Width();

	if (max_color > 24)
		max_color = 24;

	for (int i = 0; i < 25; i++) {
		sun_color[i] = Color::White;
		sky_color[i] = Color::Black;
		fog_color[i] = Color::White;
		ambient[i] = Color::DarkGray;
		cloud_color[i] = Color::White;
		shade_color[i] = Color::Gray;
		overcast[i] = Color::Black;
	}

	for (int i = 0; i < max_color; i++)
		sky_color[i] = sky_colors_bmp.GetColor(i, 0);

	if (sky_colors_bmp.Height() > 1) {
		for (int i = 0; i < max_color; i++)
			fog_color[i].Set(sky_colors_bmp.GetColor(i, 1).Value() | Color(0, 0, 0, 255).Value());
	}

	if (sky_colors_bmp.Height() > 2) {
		for (int i = 0; i < max_color; i++)
			ambient[i] = sky_colors_bmp.GetColor(i, 2);
	}

	if (sky_colors_bmp.Height() > 3) {
		for (int i = 0; i < max_color; i++)
			sun_color[i] = sky_colors_bmp.GetColor(i, 3);
	}

	if (sky_colors_bmp.Height() > 4) {
		for (int i = 0; i < max_color; i++)
			overcast[i] = sky_colors_bmp.GetColor(i, 4);
	}

	if (sky_colors_bmp.Height() > 5) {
		for (int i = 0; i < max_color; i++)
			cloud_color[i] = sky_colors_bmp.GetColor(i, 5);
	}

	if (sky_colors_bmp.Height() > 6) {
		for (int i = 0; i < max_color; i++)
			shade_color[i] = sky_colors_bmp.GetColor(i, 6);
	}
}

// +--------------------------------------------------------------------+

void
TerrainRegion::AddLayer(double h, const char* tile, const char* detail)
{
	TerrainLayer* layer = new TerrainLayer;

	layer->min_height = h;
	layer->tile_name = tile;

	if (detail && *detail)
		layer->detail_name = detail;

	layers.append(layer);
}

const Text&
TerrainRegion::EnvironmentTexture(int face) const
{
	switch (face) {
	case 0:  return env_texture_positive_x;

	case 1:
		if (env_texture_negative_x.length() > 0)
			return env_texture_negative_x;
		return env_texture_positive_x;

	case 2:  return env_texture_positive_y;

	case 3:
		if (env_texture_negative_y.length() > 0)
			return env_texture_negative_y;
		return env_texture_positive_y;

	case 4:
		if (env_texture_positive_z.length() > 0)
			return env_texture_positive_z;
		return env_texture_positive_x;

	case 5:
		if (env_texture_negative_z.length() > 0)
			return env_texture_negative_z;
		if (env_texture_positive_z.length() > 0)
			return env_texture_positive_z;
		return env_texture_positive_x;
	}

	return env_texture_positive_x;
}

