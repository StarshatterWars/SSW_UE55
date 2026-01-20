/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright (c) 2025-2026. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         ShieldRep.h
	AUTHOR:       Carlos Bott

	ORIGINAL AUTHOR AND STUDIO:
	John DiCamillo / Destroyer Studios LLC

	OVERVIEW
	========
	ShieldRep Solid class
/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright (c) 2025-2026. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         ShieldRep.cpp
	AUTHOR:       Carlos Bott

	ORIGINAL AUTHOR AND STUDIO:
	John DiCamillo / Destroyer Studios LLC

	OVERVIEW
	========
	ShieldRep Solid class
*/

#include "ShieldRep.h"

// UE logging (minimal include):
#include "Logging/LogMacros.h"

// Minimal Unreal include required by Vec3/Point -> FVector conversion:
#include "Math/Vector.h"

// Starshatter core / project headers:
#include "Random.h"
#include "Game.h"
#include "SimLight.h"
#include "Solid.h"
#include "Bitmap.h"
#include "Color.h"
#include "DataLoader.h"

// If you don't already have a log category, define a local one for this TU:
DEFINE_LOG_CATEGORY_STATIC(LogShieldRep, Log, All);

// +--------------------------------------------------------------------+

static const int MAX_SHIELD_HITS = 16;

// +--------------------------------------------------------------------+

struct ShieldHit
{
	FVector  hitloc = FVector::ZeroVector;
	double   damage = 0.0;
	double   age = 0.0;
	SimShot* shot = nullptr;
};

// +--------------------------------------------------------------------+

ShieldRep::ShieldRep()
{
	bubble = false;
	luminous = true;
	trans = true;
	nhits = 0;

	hits = new ShieldHit[MAX_SHIELD_HITS];
}

ShieldRep::~ShieldRep()
{
	delete[] hits;
	hits = nullptr;
}

// +--------------------------------------------------------------------+

void
ShieldRep::Hit(FVector impact, SimShot* shot, double damage)
{
	if (!model || model->GetSurfaces().size() < 1)
		return;

	// transform impact into object space:
	Matrix xform(Orientation());

	const FVector tmp = impact - loc;

	impact.X = tmp * FVector(xform(0, 0), xform(0, 1), xform(0, 2));
	impact.Y = tmp * FVector(xform(1, 0), xform(1, 1), xform(1, 2));
	impact.Z = tmp * FVector(xform(2, 0), xform(2, 1), xform(2, 2));

	// find slot to store the hit:
	int    slot = -1;
	double oldest_age = -1.0;

	for (int i = 0; i < MAX_SHIELD_HITS; i++) {
		if (hits[i].shot == shot) {
			slot = i;
			break;
		}
	}

	if (slot < 0) {
		for (int i = 0; i < MAX_SHIELD_HITS; i++) {
			if (hits[i].damage <= 0) {
				slot = i;
				break;
			}

			if (hits[i].age > oldest_age) {
				slot = i;
				oldest_age = hits[i].age;
			}
		}
	}

	if (slot >= 0 && slot < MAX_SHIELD_HITS) {
		// record the hit in the slot:
		hits[slot].hitloc = impact;
		hits[slot].damage = damage;
		hits[slot].age = 1.0;
		hits[slot].shot = shot;

		if (nhits < MAX_SHIELD_HITS)
			nhits++;
	}
}

// +--------------------------------------------------------------------+

void
ShieldRep::Energize(double seconds, bool b)
{
	bubble = b;

	if (nhits < 1)
		return;

	nhits = 0;

	for (int i = 0; i < MAX_SHIELD_HITS; i++) {
		if (hits[i].damage > 0) {
			// age the hit:
			hits[i].age += seconds;
			hits[i].damage -= (hits[i].damage * 4.0 * seconds);

			// collect garbage:
			if (hits[i].damage < 10) {
				hits[i].age = 0;
				hits[i].damage = 0;
				hits[i].shot = nullptr;
			}
			else {
				nhits++;
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
ShieldRep::TranslateBy(const FVector& ref)
{
	true_eye_point = ref;
	Solid::TranslateBy(ref);
}

// +--------------------------------------------------------------------+

void
ShieldRep::Illuminate()
{
	if (!model)
		return;

	Surface* surf = model->GetSurfaces().first();
	VertexSet* vset = surf->GetVertexSet();
	const int  nverts = vset->nverts;

	for (int i = 0; i < nverts; i++) {
		vset->diffuse[i] = 0;
		vset->specular[i] = 0;
	}

	double all_damage = 0.0;

	if (nhits < 1)
		return;

	for (int i = 0; i < MAX_SHIELD_HITS; i++) {
		if (hits[i].damage > 0) {
			// add the hit's contribution to the shield verts:
			const FVector hitloc = hits[i].hitloc;
			double        hitdam = hits[i].damage * 2000.0;

			all_damage += hits[i].damage;

			if (!bubble) {

				const double limit = radius * radius;
				if (hitdam > limit)
					hitdam = limit;

				for (int v = 0; v < nverts; v++) {
					double dist = (vset->loc[v] - hitloc).Length();

					if (dist < 1.0)
						dist = 1.0;  // can't divide by zero!
					else
						dist = pow(dist, 2.7);

					const double pert = FMath::FRandRange(0.1, 1.5);
					const double intensity = pert * hitdam / dist;

					if (intensity > 0.003)
						vset->diffuse[v] = ((Color::White * intensity) + vset->diffuse[v]).Value();
				}
			}
		}
	}

	if (bubble) {
		double shield_gain = 1.0;

		if (all_damage < 1000.0) {
			shield_gain = all_damage / 1000.0;
		}

		for (int i = 0; i < nverts; i++) {
			const FVector vloc = (vset->loc[i] * orientation) + loc;
			const FVector vnrm = (vset->nrm[i] * orientation);

			FVector V = vloc * -1.0f;
			V.Normalize();

			double intensity = 1.0 - V * vnrm;

			if (intensity > 0.0) {
				intensity *= intensity;

				if (intensity > 1.0)
					intensity = 1.0;

				intensity *= (shield_gain * Random(0.75, 1.0));

				const Color vs = Color::White * intensity;
				vset->diffuse[i] = vs.Value();
			}
		}
	}

	InvalidateSurfaceData();
}

void
ShieldRep::Render(Video* video, DWORD flags)
{
	if ((flags & RENDER_ADDITIVE) == 0)
		return;

	if (nhits > 0) {
		Illuminate();
		Solid::Render(video, RENDER_ALPHA); // have to lie about the render flag
		// or the engine will reject the solid
	}
}
