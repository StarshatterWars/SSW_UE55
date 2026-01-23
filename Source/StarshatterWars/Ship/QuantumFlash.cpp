/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright © 2025–2026. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         QuantumFlash.cpp
	AUTHOR:       Carlos Bott
	ORIGINAL:     John DiCamillo / Destroyer Studios LLC

	OVERVIEW
	========
	Quantum Warp Out special effect class
*/

#include "QuantumFlash.h"

// --------------------------------------------------------------------
// Starshatter / engine includes
// --------------------------------------------------------------------

#include "Graphic.h"
#include "Material.h"
#include "VertexSet.h"
#include "Polygon.h"
#include "Bitmap.h"
#include "DataLoader.h"
#include "Game.h"
#include "Random.h"
#include "SimScene.h"
#include "Video.h"
#include "Camera.h"

// --------------------------------------------------------------------
// Minimal Unreal includes
// --------------------------------------------------------------------

#include "Math/Vector.h"
#include "Math/Color.h"
#include "Math/UnrealMathUtility.h"
#include "Math/RandomStream.h"
#include "Logging/LogMacros.h"

// --------------------------------------------------------------------
// UE logging bridge (replaces Print / MemDebug)
// --------------------------------------------------------------------

DEFINE_LOG_CATEGORY_STATIC(LogQuantumFlash, Log, All);

#ifndef Print
#define Print(...) UE_LOG(LogQuantumFlash, Log, TEXT(__VA_ARGS__))
#endif

// +--------------------------------------------------------------------+

static Bitmap* quantum_flash_texture = nullptr;

// +--------------------------------------------------------------------+

QuantumFlash::QuantumFlash()
	: length(8000.0)
	, width(0.0)
	, shade(1.0)
	, npolys(16)
	, nverts(64)
	, mtl(nullptr)
	, verts(nullptr)
	, polys(nullptr)
	, beams(nullptr)
	, texture(quantum_flash_texture)
{
	trans = true;
	luminous = true;

	FRandomStream RandomStream;
	

	if (!texture || texture->Width() < 1) {
		DataLoader* loader = DataLoader::GetLoader();
		loader->SetDataPath("Explosions/");
		loader->LoadTexture("quantum.pcx", quantum_flash_texture, Bitmap::BMP_TRANSLUCENT);
		loader->SetDataPath(nullptr);
		texture = quantum_flash_texture;
	}

	loc = FVector(0.0f, 0.0f, 1000.0f);

	mtl = new Material;
	verts = new VertexSet(nverts);
	polys = new Poly[npolys];
	beams = new Matrix[npolys];

	mtl->Kd = FColor::White;
	mtl->tex_diffuse = texture;
	mtl->tex_emissive = texture;
	mtl->blend = Video::BLEND_ADDITIVE;
	mtl->luminous = true;

	verts->nverts = nverts;

	for (int i = 0; i < npolys; i++) {
		verts->loc[4 * i + 0] = FVector(width, 0.0f, 1000.0f);
		verts->loc[4 * i + 1] = FVector(width, -length, 1000.0f);
		verts->loc[4 * i + 2] = FVector(-width, -length, 1000.0f);
		verts->loc[4 * i + 3] = FVector(-width, 0.0f, 1000.0f);

		for (int n = 0; n < 4; n++) {
			verts->diffuse[4 * i + n] = FColor::White.ToPackedARGB();
			verts->specular[4 * i + n] = FColor::Black.ToPackedARGB();
			verts->tu[4 * i + n] = (n < 2) ? 0.0f : 1.0f;
			verts->tv[4 * i + n] = (n > 0 && n < 3) ? 1.0f : 0.0f;
		}

		const int32 SeedValue = i;   
		FRandomStream RandomStream(SeedValue);
		RandomStream.Initialize(SeedValue);

		beams[i].Roll(RandomStream.FRandRange(-2.0 * PI, 2.0 * PI));
		beams[i].Pitch(RandomStream.FRandRange(-2.0 * PI, 2.0 * PI));
		beams[i].Yaw(RandomStream.FRandRange(-2.0 * PI, 2.0 * PI));

		polys[i].nverts = 4;
		polys[i].visible = 1;
		polys[i].sortval = 0;
		polys[i].vertex_set = verts;
		polys[i].material = mtl;

		polys[i].verts[0] = 4 * i + 0;
		polys[i].verts[1] = 4 * i + 1;
		polys[i].verts[2] = 4 * i + 2;
		polys[i].verts[3] = 4 * i + 3;
	}

	radius = static_cast<float>(length);
	length = 0.0;

	strcpy_s(name, "QuantumFlash");
}

// +--------------------------------------------------------------------+

QuantumFlash::~QuantumFlash()
{
	delete mtl;
	delete verts;
	delete[] polys;
	delete[] beams;
}

// +--------------------------------------------------------------------+

void
QuantumFlash::Render(Video* video, DWORD flags)
{
	if (hidden || !visible || !video || ((flags & RENDER_ADDITIVE) == 0))
		return;

	const Camera* cam = video->GetCamera();
	if (cam) {
		UpdateVerts(cam->Pos());
		video->DrawPolys(npolys, polys);
	}
}

// +--------------------------------------------------------------------+

void
QuantumFlash::UpdateVerts(const FVector& cam_pos)
{
	if (length < radius) {
		length += radius / 80.0;
		width += 1.0;
	}

	for (int i = 0; i < npolys; i++) {
		Matrix& m = beams[i];

		m.Yaw(0.05);

		FVector vpn(m(2, 0), m(2, 1), m(2, 2));

		FVector head = loc;
		FVector tail = loc - vpn * length;
		FVector vtail = tail - head;
		FVector vcam = cam_pos - loc;

		FVector vtmp = FVector::CrossProduct(vcam, vtail);
		vtmp.Normalize();

		FVector vlat = vtmp * -width;

		verts->loc[4 * i + 0] = head + vlat;
		verts->loc[4 * i + 1] = tail + vlat * 8.0f;
		verts->loc[4 * i + 2] = tail - vlat * 8.0f;
		verts->loc[4 * i + 3] = head - vlat;

		FColor color(
			static_cast<uint8>(255 * shade),
			static_cast<uint8>(255 * shade),
			static_cast<uint8>(255 * shade),
			255
		);

		const DWORD packed = color.ToPackedARGB();

		for (int n = 0; n < 4; n++) {
			verts->diffuse[4 * i + n] = packed;
		}
	}
}

// +--------------------------------------------------------------------+

void
QuantumFlash::SetOrientation(const Matrix& o)
{
	orientation = o;
}

void
QuantumFlash::SetShade(double s)
{
	if (s < 0.0)      s = 0.0;
	else if (s > 1.0) s = 1.0;

	shade = s;
}
