/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright © 2025-2026. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         TerrainApron.cpp
	AUTHOR:       Carlos Bott

	ORIGINAL AUTHOR AND STUDIO:
	John DiCamillo, Destroyer Studios LLC

	OVERVIEW
	========
	A Single Edge Section of a Terrain Object
*/

#include "TerrainApron.h"

#include "Terrain.h"
#include "TerrainRegion.h"

#include "SimLight.h"
#include "SimScene.h"

// NOTE:
// Starshatter core render types remain (Model/Surface/VertexSet/Material/etc).
// Render assets previously referenced as Bitmap are now UE textures.
#include "Engine/Texture2D.h"

// Minimal Unreal includes:
#include "Math/UnrealMathUtility.h"
#include "Logging/LogMacros.h"

// +====================================================================+

const int PATCH_SIZE = 17;
const int HALF_PATCH_SIZE = 8;
const int MAX_VERTS = PATCH_SIZE * PATCH_SIZE;
const int NUM_INDICES_TRI = 3;

// +--------------------------------------------------------------------+

TerrainApron::TerrainApron(Terrain* terr, const UTexture2D* patch, const Rect& r,
	const FVector& p1, const FVector& p2)
	: terrain(terr), rect(r)
{
	size = FMath::Abs(p2.X - p1.X);
	scale = size / (PATCH_SIZE - 1);
	mtnscale = 1.3 * (p2.Y - p1.Y);
	base = p1.Y;

	// NOTE: We cannot read pixel data from UTexture2D without an explicit CPU-read path.
	// The apron heightfield was built from a CPU bitmap; if you still have that bitmap
	// pipeline in Starshatter core, keep using it there. For now, preserve behavior by
	// allocating heights and initializing to zero (flat apron).
	if (!patch) {
		UE_LOG(LogTemp, Warning, TEXT("TerrainApron: null patch texture; apron heights will be flat."));
		terrain_width = 0;
	}
	else {
		terrain_width = patch->GetSizeX();
	}

	loc = (p1 + p2) * 0.5;
	loc.Y = base;

	radius = (float)(size * 0.75);
	heights = new float[MAX_VERTS];

	// Flat fallback (0 height) unless you implement a CPU sampling path:
	for (int i = 0; i < MAX_VERTS; ++i)
		heights[i] = 0.0f;

	// If you later re-introduce CPU sampling, this is where the original loop would
	// populate heights[i*PATCH_SIZE + j] from patch pixel luminance (Red channel).
}

// +--------------------------------------------------------------------+

TerrainApron::~TerrainApron()
{
	delete[] heights;
	heights = nullptr;
}

// +--------------------------------------------------------------------+

void TerrainApron::SetScales(double s, double m, double b)
{
	scale = s;
	mtnscale = m;
	base = b;
}

// +--------------------------------------------------------------------+

bool TerrainApron::BuildApron()
{
	int detail_size = PATCH_SIZE - 1;
	int ds1 = PATCH_SIZE;
	int nverts = MAX_VERTS;
	int npolys = detail_size * detail_size * 2;

	model = new Model;
	model->SetLuminous(true);
	model->SetDynamic(true);

	Material* mtl = new Material;
	mtl->Ka = ColorValue(0.5f, 0.5f, 0.5f);
	mtl->Kd = ColorValue(0.3f, 0.6f, 0.2f);
	mtl->Ks = Color::Black;

	// Terrain::ApronTexture() should now return UTexture2D* (or be adapted accordingly).
	// Material expects "tex_diffuse" to be the Starshatter core type; keep the hook here
	// consistent with your engine-side bridge.
	mtl->tex_diffuse = terrain ? terrain->ApronTexture() : nullptr;
	strcpy_s(mtl->name, "Terrain Apron");

	model->GetMaterials().append(mtl);

	Surface* s = new Surface;
	VertexSet* vset = nullptr;

	if (s) {
		s->SetName("default");
		s->CreateVerts(nverts);
		s->CreatePolys(npolys);
		s->AddIndices(npolys * NUM_INDICES_TRI);

		vset = s->GetVertexSet();

		// VertexSet uses Starshatter core vectors; convert Point/Vec3 usage to FVector per project rule.
		// These arrays are still plain POD buffers; preserve ZeroMemory behavior via memset.
		memset(vset->loc, 0, nverts * sizeof(FVector));
		memset(vset->diffuse, 0, nverts * sizeof(DWORD));
		memset(vset->specular, 0, nverts * sizeof(DWORD));
		memset(vset->tu, 0, nverts * sizeof(float));
		memset(vset->tv, 0, nverts * sizeof(float));
		memset(vset->rw, 0, nverts * sizeof(float));

		// initialize vertices
		FVector* pVert = vset->loc;
		float* pTu = vset->tu;
		float* pTv = vset->tv;

		const double dt = (1.0 / 3.0) / (double)detail_size;
		const double tu0 = (double)rect.x / rect.w / 3.0 + (1.0 / 3.0);
		const double tv0 = (double)rect.y / rect.h / 3.0;

		for (int i = 0; i < ds1; i++) {
			for (int j = 0; j < ds1; j++) {
				*pVert++ = FVector(
					(float)(j * scale - (HALF_PATCH_SIZE * scale)),
					(float)(heights[i * PATCH_SIZE + j]),
					(float)(i * scale - (HALF_PATCH_SIZE * scale)));

				*pTu++ = (float)(tu0 - j * dt);
				*pTv++ = (float)(tv0 + i * dt);
			}
		}

		// create the polys
		for (int i = 0; i < npolys; i++) {
			Poly* p = s->GetPolys() + i;
			p->nverts = 3;
			p->vertex_set = vset;
			p->material = mtl;
			p->visible = 1;
			p->sortval = 0;
		}

		int index = 0;

		// build main patch polys:
		for (int i = 0; i < detail_size; i++) {
			for (int j = 0; j < detail_size; j++) {
				// first triangle
				Poly* p = s->GetPolys() + index++;
				p->verts[0] = (ds1 * (i)+(j));
				p->verts[1] = (ds1 * (i)+(j + 1));
				p->verts[2] = (ds1 * (i + 1) + (j + 1));

				// second triangle
				p = s->GetPolys() + index++;
				p->verts[0] = (ds1 * (i)+(j));
				p->verts[1] = (ds1 * (i + 1) + (j + 1));
				p->verts[2] = (ds1 * (i + 1) + (j));
			}
		}

		// update the verts and colors of each poly:
		for (int i = 0; i < npolys; i++) {
			Poly* p = s->GetPolys() + i;
			Plane& plane = p->plane;
			WORD* v = p->verts;

			plane = Plane(vset->loc[v[0]] + loc,
				vset->loc[v[1]] + loc,
				vset->loc[v[2]] + loc);
		}

		// create contiguous segments for each material:
		s->Normalize();

		Segment* segment = new Segment(npolys, s->GetPolys(), mtl, model);
		s->GetSegments().append(segment);

		model->AddSurface(s);

		// copy vertex normals:
		const FVector normal(0.f, 1.f, 0.f);

		for (int i = 0; i < ds1; i++) {
			for (int j = 0; j < ds1; j++) {
				vset->nrm[i * ds1 + j] = normal;
			}
		}
	}

	return true;
}

// +--------------------------------------------------------------------+

int TerrainApron::CollidesWith(Graphic& o)
{
	return 0;
}

// +--------------------------------------------------------------------+

void TerrainApron::Update()
{
}

// +--------------------------------------------------------------------+

void TerrainApron::Illuminate(FColor ambient, List<SimLight>& lights)
{
	if (!model || model->NumVerts() < 1)
		return;

	Surface* s = model->GetSurfaces().first();
	if (!s)
		return;

	// clear the solid lights to ambient:
	VertexSet* vset = s->GetVertexSet();
	if (!vset || vset->nverts < 1)
		return;

	const int    nverts = vset->nverts;

	// FColor has no Value(); store packed ARGB (0xAARRGGBB):
	const uint32 aval = ambient.ToPackedARGB();

	for (int i = 0; i < nverts; i++) {
		vset->diffuse[i] = (DWORD)aval;
	}

	bool eclipsed = false;

	// Helpers for packed diffuse accumulation (assumes 0xAARRGGBB):
	auto UnpackARGB = [](uint32 Packed) -> FColor
		{
			return FColor(
				(uint8)((Packed >> 16) & 0xFF), // R
				(uint8)((Packed >> 8) & 0xFF), // G
				(uint8)((Packed >> 0) & 0xFF), // B
				(uint8)((Packed >> 24) & 0xFF)  // A
			);
		};

	auto PackARGB = [](const FColor& C) -> uint32
		{
			return (uint32(C.A) << 24) | (uint32(C.R) << 16) | (uint32(C.G) << 8) | uint32(C.B);
		};

	auto ScaleRGB = [](const FColor& C, float S) -> FColor
		{
			return FColor(
				(uint8)FMath::Clamp(int32(C.R * S), 0, 255),
				(uint8)FMath::Clamp(int32(C.G * S), 0, 255),
				(uint8)FMath::Clamp(int32(C.B * S), 0, 255),
				0 // do not add alpha during lighting
			);
		};

	auto AddClampRGB = [](const FColor& A, const FColor& B) -> FColor
		{
			return FColor(
				(uint8)FMath::Clamp(int32(A.R) + int32(B.R), 0, 255),
				(uint8)FMath::Clamp(int32(A.G) + int32(B.G), 0, 255),
				(uint8)FMath::Clamp(int32(A.B) + int32(B.B), 0, 255),
				A.A
			);
		};

	// for each light:
	ListIter<SimLight> iter = lights;
	while (++iter) {
		SimLight* light = iter.value();
		if (!light)
			continue;

		if (light->CastsShadow())
			eclipsed = light->Location().Y < -100.0f;

		if (!light->CastsShadow() || !eclipsed) {
			FVector vl = light->Location().GetSafeNormal();

			for (int i = 0; i < nverts; i++) {
				const FVector& nrm = vset->nrm[i];

				double val = 0.0;

				if (light->IsDirectional()) {
					const double gain = (double)FVector::DotProduct(vl, nrm);

					if (gain > 0.0) {
						val = light->Intensity() * (0.85 * gain);
						if (val > 1.0) val = 1.0;
					}
				}

				if (val > 0.01) {
					const FColor Base = UnpackARGB((uint32)vset->diffuse[i]);

					const FColor Add = ScaleRGB(light->GetColor(), (float)val);
					const FColor Out = AddClampRGB(Base, Add);

					vset->diffuse[i] = (DWORD)PackARGB(Out);
				}
			}
		}
	}

	InvalidateSurfaceData();
}

// +--------------------------------------------------------------------+

void TerrainApron::Render(Video* video, DWORD flags)
{
	if (!video || (flags & RENDER_ADDITIVE) || (flags & RENDER_ADD_LIGHT))
		return;

	if (!model)
		BuildApron();

	if (scene) {
		Illuminate(scene->Ambient(), scene->Lights());
	}

	const double visibility = terrain->GetRegion()->GetWeather().Visibility();
	const float  fog_density = (float)(terrain->GetRegion()->FogDensity() * 2.5e-5 * 1.0 / visibility);

	video->SetRenderState(Video::LIGHTING_ENABLE, false);
	video->SetRenderState(Video::SPECULAR_ENABLE, false);
	video->SetRenderState(Video::FOG_ENABLE, true);

	const FColor Fog = terrain->GetRegion()->FogColor();

	const DWORD FogPacked =
		(DWORD(Fog.A) << 24) |
		(DWORD(Fog.R) << 16) |
		(DWORD(Fog.G) << 8) |
		(DWORD(Fog.B));

	video->SetRenderState(Video::FOG_COLOR, FogPacked);
	video->SetRenderState(Video::FOG_DENSITY, *((DWORD*)&fog_density));

	Solid::Render(video, flags);

	video->SetRenderState(Video::LIGHTING_ENABLE, true);
	video->SetRenderState(Video::SPECULAR_ENABLE, true);
	video->SetRenderState(Video::FOG_ENABLE, false);
}

// +--------------------------------------------------------------------+

int TerrainApron::CheckRayIntersection(FVector Q, FVector w, double len, FVector& ipt, bool ttpas)
{
	// compute leading edge of ray:
	FVector sun = Q + w * (float)len;

	if (sun.Y < loc.Y)
		return 1;

	return 0;
}

