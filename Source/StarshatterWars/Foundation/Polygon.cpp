/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright (C) 2025-2026. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         Polygon.cpp
	AUTHOR:       Carlos Bott
	ORIGINAL:     John DiCamillo / Destroyer Studios LLC

	OVERVIEW
	========
	Polygon and VertexSet structures for 3D rendering
*/

#include "Polygon.h"

// Minimal Unreal includes:
#include "Math/Vector.h"
#include "HAL/UnrealMemory.h"
#include "Logging/LogMacros.h"

#include <cmath>
#include <cstring>

DEFINE_LOG_CATEGORY_STATIC(LogStarshatterPolygon, Log, All);

// +--------------------------------------------------------------------+

VertexSet::VertexSet(int m)
	: nverts(0), space(OBJECT_SPACE),
	loc(0), nrm(0), s_loc(0),
	rw(0), tu(0), tv(0), tu1(0), tv1(0),
	diffuse(0), specular(0),
	tangent(0), binormal(0)
{
	Resize(m);
}

// +--------------------------------------------------------------------+

VertexSet::~VertexSet()
{
	Delete();
}

// +--------------------------------------------------------------------+

void
VertexSet::Resize(int m, bool preserve)
{
	// easy cases (no data will be preserved):
	if (!m || !nverts || !preserve) {
		const bool additional_tex_coords = (tu1 != 0);

		Delete();
		nverts = m;

		if (nverts <= 0) {
			FMemory::Memzero(this, sizeof(VertexSet));
		}
		else {
			loc = new FVector[nverts];
			nrm = new FVector[nverts];
			s_loc = new FVector[nverts];
			tu = new float[nverts];
			tv = new float[nverts];
			rw = new float[nverts];
			diffuse = new DWORD[nverts];
			specular = new DWORD[nverts];

			if (additional_tex_coords)
				CreateAdditionalTexCoords();

			if (!loc || !nrm || !s_loc || !rw || !tu || !tv || !diffuse || !specular) {
				nverts = 0;

				delete[] loc;      loc = 0;
				delete[] nrm;      nrm = 0;
				delete[] s_loc;    s_loc = 0;
				delete[] rw;       rw = 0;
				delete[] tu;       tu = 0;
				delete[] tv;       tv = 0;
				delete[] tu1;      tu1 = 0;
				delete[] tv1;      tv1 = 0;
				delete[] diffuse;  diffuse = 0;
				delete[] specular; specular = 0;

				FMemory::Memzero(this, sizeof(VertexSet));
			}
		}
	}

	// actually need to copy data:
	else {
		int np = nverts;
		nverts = m;

		if (nverts < np)
			np = nverts;

		FVector* new_loc = new FVector[nverts];
		FVector* new_nrm = new FVector[nverts];
		FVector* new_s_loc = new FVector[nverts];
		float* new_rw = new float[nverts];
		float* new_tu = new float[nverts];
		float* new_tv = new float[nverts];
		float* new_tu1 = 0;
		float* new_tv1 = 0;
		DWORD* new_diffuse = new DWORD[nverts];
		DWORD* new_specular = new DWORD[nverts];

		if (tu1) new_tu1 = new float[nverts];
		if (tv1) new_tv1 = new float[nverts];

		if (new_loc) {
			FMemory::Memcpy(new_loc, loc, np * sizeof(FVector));
			delete[] loc;
			loc = new_loc;
		}

		if (new_nrm) {
			FMemory::Memcpy(new_nrm, nrm, np * sizeof(FVector));
			delete[] nrm;
			nrm = new_nrm;
		}

		if (new_s_loc) {
			FMemory::Memcpy(new_s_loc, s_loc, np * sizeof(FVector));
			delete[] s_loc;
			s_loc = new_s_loc;
		}

		if (new_rw) {
			FMemory::Memcpy(new_rw, rw, np * sizeof(float));
			delete[] rw;
			rw = new_rw;
		}

		if (new_tu) {
			FMemory::Memcpy(new_tu, tu, np * sizeof(float));
			delete[] tu;
			tu = new_tu;
		}

		if (new_tv) {
			FMemory::Memcpy(new_tv, tv, np * sizeof(float));
			delete[] tv;
			tv = new_tv;
		}

		// NOTE: fixed legacy typos (tu/tv were incorrectly assigned):
		if (new_tu1) {
			FMemory::Memcpy(new_tu1, tu1, np * sizeof(float));
			delete[] tu1;
			tu1 = new_tu1;
		}

		if (new_tv1) {
			FMemory::Memcpy(new_tv1, tv1, np * sizeof(float));
			delete[] tv1;
			tv1 = new_tv1;
		}

		if (new_diffuse) {
			FMemory::Memcpy(new_diffuse, diffuse, np * sizeof(DWORD));
			delete[] diffuse;
			diffuse = new_diffuse;
		}

		if (new_specular) {
			FMemory::Memcpy(new_specular, specular, np * sizeof(DWORD));
			delete[] specular;
			specular = new_specular;
		}

		if (!loc || !nrm || !s_loc || !rw || !tu || !tv || !diffuse || !specular) {
			Delete();
			FMemory::Memzero(this, sizeof(VertexSet));
		}
	}
}

// +--------------------------------------------------------------------+

void
VertexSet::Delete()
{
	if (nverts) {
		delete[] loc;      loc = 0;
		delete[] nrm;      nrm = 0;
		delete[] s_loc;    s_loc = 0;
		delete[] rw;       rw = 0;
		delete[] tu;       tu = 0;
		delete[] tv;       tv = 0;
		delete[] tu1;      tu1 = 0;
		delete[] tv1;      tv1 = 0;
		delete[] diffuse;  diffuse = 0;
		delete[] specular; specular = 0;
		delete[] tangent;  tangent = 0;
		delete[] binormal; binormal = 0;

		nverts = 0;
	}
}

// +--------------------------------------------------------------------+

void
VertexSet::Clear()
{
	if (nverts) {
		FMemory::Memzero(loc, sizeof(FVector) * nverts);
		FMemory::Memzero(nrm, sizeof(FVector) * nverts);
		FMemory::Memzero(s_loc, sizeof(FVector) * nverts);
		FMemory::Memzero(tu, sizeof(float) * nverts);
		FMemory::Memzero(tv, sizeof(float) * nverts);
		FMemory::Memzero(rw, sizeof(float) * nverts);
		FMemory::Memzero(diffuse, sizeof(DWORD) * nverts);
		FMemory::Memzero(specular, sizeof(DWORD) * nverts);

		if (tu1)      FMemory::Memzero(tu1, sizeof(float) * nverts);
		if (tv1)      FMemory::Memzero(tv1, sizeof(float) * nverts);
		if (tangent)  FMemory::Memzero(tangent, sizeof(FVector) * nverts);
		if (binormal) FMemory::Memzero(binormal, sizeof(FVector) * nverts);
	}
}

// +--------------------------------------------------------------------+

void
VertexSet::CreateTangents()
{
	delete[] tangent;  tangent = 0;
	delete[] binormal; binormal = 0;

	if (nverts) {
		tangent = new FVector[nverts];
		binormal = new FVector[nverts];
	}
}

// +--------------------------------------------------------------------+

void
VertexSet::CreateAdditionalTexCoords()
{
	delete[] tu1; tu1 = 0;
	delete[] tv1; tv1 = 0;

	if (nverts) {
		tu1 = new float[nverts];
		tv1 = new float[nverts];
	}
}

// +--------------------------------------------------------------------+

bool
VertexSet::CopyVertex(int dst, int src)
{
	if (src >= 0 && src < nverts && dst >= 0 && dst < nverts) {
		loc[dst] = loc[src];
		nrm[dst] = nrm[src];
		s_loc[dst] = s_loc[src];
		rw[dst] = rw[src];
		tu[dst] = tu[src];
		tv[dst] = tv[src];
		diffuse[dst] = diffuse[src];
		specular[dst] = specular[src];

		if (tu1)      tu1[dst] = tu1[src];
		if (tv1)      tv1[dst] = tv1[src];
		if (tangent)  tangent[dst] = tangent[src];
		if (binormal) binormal[dst] = binormal[src];

		return true;
	}

	return false;
}

VertexSet*
VertexSet::Clone() const
{
	VertexSet* result = new VertexSet(nverts);

	FMemory::Memcpy(result->loc, loc, nverts * sizeof(FVector));
	FMemory::Memcpy(result->nrm, nrm, nverts * sizeof(FVector));
	FMemory::Memcpy(result->s_loc, s_loc, nverts * sizeof(FVector));
	FMemory::Memcpy(result->rw, rw, nverts * sizeof(float));
	FMemory::Memcpy(result->tu, tu, nverts * sizeof(float));
	FMemory::Memcpy(result->tv, tv, nverts * sizeof(float));
	FMemory::Memcpy(result->diffuse, diffuse, nverts * sizeof(DWORD));
	FMemory::Memcpy(result->specular, specular, nverts * sizeof(DWORD));

	if (tu1) {
		if (!result->tu1)
			result->tu1 = new float[nverts];

		FMemory::Memcpy(result->tu1, tu1, nverts * sizeof(float));
	}

	if (tv1) {
		if (!result->tv1)
			result->tv1 = new float[nverts];

		FMemory::Memcpy(result->tv1, tv1, nverts * sizeof(float));
	}

	if (tangent) {
		if (!result->tangent)
			result->tangent = new FVector[nverts];

		FMemory::Memcpy(result->tangent, tangent, nverts * sizeof(FVector));
	}

	if (binormal) {
		if (!result->binormal)
			result->binormal = new FVector[nverts];

		FMemory::Memcpy(result->binormal, binormal, nverts * sizeof(FVector));
	}

	return result;
}

void
VertexSet::CalcExtents(FVector& plus, FVector& minus)
{
	plus = FVector(-1e6f, -1e6f, -1e6f);
	minus = FVector(1e6f, 1e6f, 1e6f);

	for (int i = 0; i < nverts; i++) {
		if (loc[i].X > plus.X)   plus.X = loc[i].X;
		if (loc[i].X < minus.X)  minus.X = loc[i].X;

		if (loc[i].Y > plus.Y)   plus.Y = loc[i].Y;
		if (loc[i].Y < minus.Y)  minus.Y = loc[i].Y;

		if (loc[i].Z > plus.Z)   plus.Z = loc[i].Z;
		if (loc[i].Z < minus.Z)  minus.Z = loc[i].Z;
	}
}

// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+

Poly::Poly(int init)
	: nverts(0), visible(1), vertex_set(0), material(0), sortval(0), flatness(0)
{
}

// +--------------------------------------------------------------------+
// Check to see if a test point is within the bounds of the poly.
// The point is assumed to be coplanar with the poly.  Return 1 if
// the point is inside, 0 if the point is outside.

Vec2 projverts[Poly::MAX_VERTS];

static inline double extent3(double a, double b, double c)
{
	const double d1 = fabs(a - b);
	const double d2 = fabs(a - c);
	const double d3 = fabs(b - c);

	if (d1 > d2) {
		if (d1 > d3) return d1;
		return d3;
	}
	else {
		if (d2 > d3) return d2;
		return d3;
	}
}

int
Poly::Contains(const FVector& pt) const
{
	// find largest 2d projection of this 3d Poly:
	int projaxis;

	const double pnx = fabs(plane.normal.x);
	const double pny = fabs(plane.normal.y);
	const double pnz = fabs(plane.normal.z);

	if (pnx > pny)
		if (pnx > pnz)
			if (plane.normal.x > 0) projaxis = 1;
			else                    projaxis = -1;
		else
			if (plane.normal.z > 0) projaxis = 3;
			else                    projaxis = -3;
	else
		if (pny > pnz)
			if (plane.normal.y > 0) projaxis = 2;
			else                    projaxis = -2;
		else
			if (plane.normal.z > 0) projaxis = 3;
			else                    projaxis = -3;

	for (int i = 0; i < nverts; i++) {
		const FVector loc3 = vertex_set->loc[verts[i]];
		switch (projaxis) {
		case  1: projverts[i] = Vec2(loc3.Y, loc3.Z); break;
		case -1: projverts[i] = Vec2(loc3.Z, loc3.Y); break;
		case  2: projverts[i] = Vec2(loc3.Z, loc3.X); break;
		case -2: projverts[i] = Vec2(loc3.X, loc3.Z); break;
		case  3: projverts[i] = Vec2(loc3.X, loc3.Y); break;
		case -3: projverts[i] = Vec2(loc3.Y, loc3.X); break;
		}
	}

	// now project the test point into the same plane:
	Vec2 test;
	switch (projaxis) {
	case  1: test.x = pt.Y; test.y = pt.Z; break;
	case -1: test.x = pt.Z; test.y = pt.Y; break;
	case  2: test.x = pt.Z; test.y = pt.X; break;
	case -2: test.x = pt.X; test.y = pt.Z; break;
	case  3: test.x = pt.X; test.y = pt.Y; break;
	case -3: test.x = pt.Y; test.y = pt.X; break;
	}

	const float INSIDE_EPSILON = -0.01f;

	// if the test point is outside of any segment, it is outside the entire convex Poly.
	for (int i = 0; i < nverts - 1; i++) {
		if (verts[i] != verts[i + 1]) {
			const Vec2 segment = projverts[i + 1] - projverts[i];
			const Vec2 segnorm = segment.normal();
			const Vec2 tdelta = projverts[i] - test;
			const float inside = segnorm * tdelta;
			if (inside < INSIDE_EPSILON)
				return 0;
		}
	}

	// check last segment, too:
	if (verts[0] != verts[nverts - 1]) {
		const Vec2 segment = projverts[0] - projverts[nverts - 1];
		const float inside = segment.normal() * (projverts[0] - test);
		if (inside < INSIDE_EPSILON)
			return 0;
	}

	// still here? must be inside:
	return 1;
}

// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+

Material::Material()
	: power(1.0f), brilliance(1.0f), bump(0.0f), blend(MTL_SOLID),
	shadow(true), luminous(false),
	tex_diffuse(0), tex_specular(0), tex_bumpmap(0), tex_emissive(0),
	tex_alternate(0), tex_detail(0),
	thumbnail(0),
	ambient_value(0.2f), diffuse_value(1.0f), specular_value(0.0f), emissive_value(0.0f)
{
	FMemory::Memzero(name, sizeof(name));
	FMemory::Memzero(shader, sizeof(shader));
}

// +--------------------------------------------------------------------+

Material::~Material()
{
	// UTexture2D assets are owned/managed by Unreal (GC). Never delete here.
	tex_diffuse = 0;
	tex_specular = 0;
	tex_bumpmap = 0;
	tex_emissive = 0;
	tex_alternate = 0;
	tex_detail = 0;
	thumbnail = 0;
}

// +--------------------------------------------------------------------+

int
Material::operator == (const Material& m) const
{
	if (this == &m)                        return 1;

	if (Ka != m.Ka)             return 0;
	if (Kd != m.Kd)             return 0;
	if (Ks != m.Ks)             return 0;
	if (Ke != m.Ke)             return 0;
	if (power != m.power)          return 0;
	if (brilliance != m.brilliance)     return 0;
	if (bump != m.bump)           return 0;
	if (blend != m.blend)          return 0;
	if (shadow != m.shadow)         return 0;
	if (tex_diffuse != m.tex_diffuse)    return 0;
	if (tex_specular != m.tex_specular)   return 0;
	if (tex_bumpmap != m.tex_bumpmap)    return 0;
	if (tex_emissive != m.tex_emissive)   return 0;
	if (tex_alternate != m.tex_alternate)  return 0;
	if (tex_detail != m.tex_detail)     return 0;

	return !strcmp(name, m.name);
}

// +--------------------------------------------------------------------+

void
Material::Clear()
{
	Ka = ColorValue();
	Kd = ColorValue();
	Ks = ColorValue();
	Ke = ColorValue();

	power = 1.0f;
	bump = 0.0f;
	blend = MTL_SOLID;
	shadow = true;

	tex_diffuse = 0;
	tex_specular = 0;
	tex_bumpmap = 0;
	tex_emissive = 0;
	tex_alternate = 0;
	tex_detail = 0;
	thumbnail = 0;
}

// +--------------------------------------------------------------------+

static char shader_name[Material::NAMELEN];

const char*
Material::GetShader(int pass) const
{
	int level = 0;
	if (pass > 1) pass--;

	for (int i = 0; i < NAMELEN; i++) {
		if (shader[i] == '/') {
			level++;
			if (level > pass)
				return 0;
		}
		else if (shader[i] != 0) {
			if (level == pass) {
				FMemory::Memzero(shader_name, NAMELEN);

				char* s = shader_name;
				while (i < NAMELEN && shader[i] != 0 && shader[i] != '/') {
					*s++ = shader[i++];
				}

				return shader_name;
			}
		}
		else {
			return 0;
		}
	}

	return 0;
}

// +--------------------------------------------------------------------+

void
Material::CreateThumbnail(int size)
{
	// In classic Starshatter, this generated a software Bitmap.
	// In Unreal, thumbnail generation should be handled by an engine-side utility
	// that creates/updates a UTexture2D in a UObject context.
	//
	// Keep API compatibility but avoid unsafe UObject creation here.
	UE_LOG(LogStarshatterPolygon, Verbose,
		TEXT("Material::CreateThumbnail(%d) ignored (UTexture2D creation not supported in plain C++ Material)."),
		size);

	thumbnail = 0;
}

DWORD
Material::GetThumbColor(int i, int j, int size)
{
	// Preserve the analytic lighting behavior, but do not sample textures here.
	// Texture sampling would require explicit engine helpers (bulk data read, SRGB, etc.).
	Color result = Color::LightGray;

	const double x = i - size / 2;
	const double y = j - size / 2;
	const double r = 0.9 * size / 2;
	const double d = sqrt(x * x + y * y);

	if (d <= r) {
		const double z = sqrt(r * r - x * x - y * y);

		FVector loc((float)x, (float)y, (float)z);
		FVector nrm = loc;
		nrm.Normalize();

		FVector light(1.f, -1.f, 1.f);
		light.Normalize();

		const FVector eye(0.f, 0.f, 1.f);

		ColorValue c = Ka * ColorValue(0.25f, 0.25f, 0.25f); // ambient light
		const ColorValue white(1, 1, 1);

		double diffuse = (double)FVector::DotProduct(nrm, light);

		ColorValue cd = Kd;
		ColorValue cs = Ks;
		ColorValue ce = Ke;

		// anisotropic diffuse lighting
		if (brilliance >= 0) {
			diffuse = pow(diffuse, (double)brilliance);
		}

		// forward lighting
		if (diffuse > 0) {
			// diffuse
			c += cd * (white * (float)diffuse);

			// specular
			if (power > 0) {
				const double nl = (double)FVector::DotProduct(nrm, light);
				const FVector rvec = (nrm * (float)(2.0 * nl)) - light;
				double spec = (double)FVector::DotProduct(rvec, eye);

				if (spec > 0.01) {
					spec = pow(spec, (double)power);
					c += cs * (white * (float)spec);
				}
			}
		}

		// back lighting
		else {
			diffuse *= -0.5;
			c += cd * (white * (float)diffuse);

			// specular
			if (power > 0) {
				FVector l2 = -light;

				const double nl = (double)FVector::DotProduct(nrm, l2);
				const FVector rvec = (nrm * (float)(2.0 * nl)) - l2;
				double spec = (double)FVector::DotProduct(rvec, eye);

				if (spec > 0.01) {
					spec = pow(spec, (double)power);
					c += cs * (white * (float)spec) * 0.7f;
				}
			}
		}

		c += ce;
		result = c.ToColor();
	}

	return result.Value();
}
