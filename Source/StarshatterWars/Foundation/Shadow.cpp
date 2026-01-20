/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright 2025-2026. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         Shadow.cpp
	AUTHOR:       Carlos Bott

	ORIGINAL AUTHOR AND STUDIO:
	John DiCamillo, Destroyer Studios LLC

	OVERVIEW
	========
	Dynamic Stencil Shadow Volumes
*/

#include "Shadow.h"

#include "Types.h"
#include "Geometry.h"
#include "Color.h"

// Minimal Unreal includes:
#include "Math/Vector.h"
#include "Logging/LogMacros.h"

// Concrete type includes (forward-declared in header):
#include "Solid.h"
#include "Video.h"
#include "SimLight.h"
#include "SimScene.h"

DEFINE_LOG_CATEGORY_STATIC(LogStarshatterShadow, Log, All);

static bool visible_shadow_volumes = false;

// +--------------------------------------------------------------------+

Shadow::Shadow(Solid* s)
	: solid(s)
	, verts(nullptr)
	, nverts(0)
	, max_verts(0)
	, enabled(true)
	, edges(nullptr)
	, num_edges(0)
{
	if (solid && solid->GetModel()) {
		Model* model = solid->GetModel();
		const int npolys = model->NumPolys();

		max_verts = model->NumVerts() * 4;

		verts = new FVector[max_verts];
		edges = new WORD[npolys * 6];
	}
}

// +--------------------------------------------------------------------+

Shadow::~Shadow()
{
	delete[] verts;
	verts = nullptr;

	delete[] edges;
	edges = nullptr;
}

// +--------------------------------------------------------------------+

void Shadow::Reset()
{
	num_edges = 0;
	nverts = 0;
}

// +--------------------------------------------------------------------+

void Shadow::Render(Video* video)
{
	if (!video || !solid)
		return;

	if (enabled) {
		video->DrawShadow(solid, nverts, verts, visible_shadow_volumes);
	}
}

// +--------------------------------------------------------------------+

static inline FVector TransformVectorByBasisRows(const FVector& v, const Matrix& m)
{
	// Starshatter Matrix stores basis rows accessed via m(r,c).
	// Original code computed:
	//   out.x = v * Vec3(m00,m01,m02)  (dot with row 0)
	//   out.y = v * Vec3(m10,m11,m12)  (dot with row 1)
	//   out.z = v * Vec3(m20,m21,m22)  (dot with row 2)
	return FVector(
		(float)(v.X * m(0, 0) + v.Y * m(0, 1) + v.Z * m(0, 2)),
		(float)(v.X * m(1, 0) + v.Y * m(1, 1) + v.Z * m(1, 2)),
		(float)(v.X * m(2, 0) + v.Y * m(2, 1) + v.Z * m(2, 2))
	);
}

void Shadow::Update(SimLight* light)
{
	Reset();

	if (!light || !solid || !solid->GetModel() || !edges)
		return;

	FVector lpos = light->Location();
	const bool directional = light->Type() == SimLight::LIGHT_DIRECTIONAL;
	Model* model = solid->GetModel();

	ListIter<Surface> iter = model->GetSurfaces();
	while (++iter) {
		Surface* surf = iter.value();
		if (!surf)
			continue;

		// Transform light location into surface object space:
		Matrix xform(solid->Orientation()); 

		FVector tmp = light->Location();

		if (!directional)
			tmp -= (solid->Location() + surf->GetOffset());

		lpos = TransformVectorByBasisRows(tmp, xform);

		// Compute the silhouette for the mesh with respect to the light:
		for (int i = 0; i < surf->NumPolys(); i++) {
			Poly* p = surf->GetPolys() + i;
			if (!p)
				continue;

			// Skip polys with non-shadowing materials:
			if (p->material && !p->material->shadow)
				continue;

			// If this poly faces the light:
			// NOTE: This assumes your plane.normal is now FVector (per conversion rule).
			if (FVector::DotProduct(p->plane.normal, lpos) > 0.0f) {
				for (int n = 0; n < p->nverts; n++) {
					if (n < p->nverts - 1)
						AddEdge(p->vlocs[n], p->vlocs[n + 1]);
					else
						AddEdge(p->vlocs[n], p->vlocs[0]);
				}
			}
		}

		// Extrude the silhouette away from the light source to create the shadow volume:
		FVector extent = -lpos;
		extent.Normalize();
		extent *= 50.0e3f; // solid->Radius() * 2.1f;

		for (int i = 0; i < (int)num_edges; i++) {
			if (nverts + 6 <= max_verts) {
				// NOTE: Assumes Surface::GetVLoc() now returns FVector* after Vec3->FVector conversion.
				const FVector v1 = surf->GetVLoc()[edges[2 * i + 0]];
				const FVector v2 = surf->GetVLoc()[edges[2 * i + 1]];
				const FVector v3 = v1 + extent;
				const FVector v4 = v2 + extent;

				verts[nverts++] = v1;
				verts[nverts++] = v2;
				verts[nverts++] = v3;

				verts[nverts++] = v2;
				verts[nverts++] = v4;
				verts[nverts++] = v3;
			}
			else {
				UE_LOG(LogStarshatterShadow, Warning, TEXT("Shadow volume vertex buffer overflow (nverts=%d max=%d)"), nverts, max_verts);
				break;
			}
		}
	}
}

// +--------------------------------------------------------------------+

void Shadow::AddEdge(WORD v1, WORD v2)
{
	// Remove interior edges (which appear in the list twice)
	for (DWORD i = 0; i < num_edges; i++) {
		if ((edges[2 * i + 0] == v1 && edges[2 * i + 1] == v2) ||
			(edges[2 * i + 0] == v2 && edges[2 * i + 1] == v1))
		{
			if (num_edges > 1) {
				edges[2 * i + 0] = edges[2 * (num_edges - 1) + 0];
				edges[2 * i + 1] = edges[2 * (num_edges - 1) + 1];
			}

			num_edges--;
			return;
		}
	}

	edges[2 * num_edges + 0] = v1;
	edges[2 * num_edges + 1] = v2;

	num_edges++;
}

// +--------------------------------------------------------------------+

bool Shadow::GetVisibleShadowVolumes()
{
	return visible_shadow_volumes;
}

void Shadow::SetVisibleShadowVolumes(bool vis)
{
	visible_shadow_volumes = vis;
}
