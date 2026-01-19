/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright (C) 2025-2026. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         SimScene.cpp
	AUTHOR:       Carlos Bott
	ORIGINAL:     John DiCamillo / Destroyer Studios LLC

	OVERVIEW
	========
	A 3D Scene
*/

#include "SimScene.h"
#include "Graphic.h"
#include "SimLight.h"

// Minimal Unreal includes:
#include "Math/Vector.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogStarshatterSimScene, Log, All);

// +--------------------------------------------------------------------+

SimScene::SimScene()
{
}

SimScene::~SimScene()
{
	background.destroy();
	foreground.destroy();
	graphics.destroy();
	sprites.destroy();
	lights.destroy();
}

// +--------------------------------------------------------------------+

void
SimScene::AddBackground(Graphic* g)
{
	if (g) {
		if (!background.contains(g))
			background.append(g);

		g->SetScene(this);
	}
}

void
SimScene::DelBackground(Graphic* g)
{
	if (g) {
		background.remove(g);
		g->SetScene(0);
	}
}

// +--------------------------------------------------------------------+

void
SimScene::AddForeground(Graphic* g)
{
	if (g) {
		if (!foreground.contains(g))
			foreground.append(g);

		g->SetScene(this);
	}
}

void
SimScene::DelForeground(Graphic* g)
{
	if (g) {
		foreground.remove(g);
		g->SetScene(0);
	}
}

// +--------------------------------------------------------------------+

void
SimScene::AddGraphic(Graphic* g)
{
	if (g) {
		if (!graphics.contains(g))
			graphics.append(g);

		g->SetScene(this);
	}
}

void
SimScene::DelGraphic(Graphic* g)
{
	if (g) {
		graphics.remove(g) || // it's gotta be in here somewhere!
			foreground.remove(g) || // use the logical-or operator to early
			sprites.remove(g) || // out when we find it...
			background.remove(g);

		g->SetScene(0);
	}
}

// +--------------------------------------------------------------------+

void
SimScene::AddSprite(Graphic* g)
{
	if (g) {
		if (!sprites.contains(g))
			sprites.append(g);

		g->SetScene(this);
	}
}

void
SimScene::DelSprite(Graphic* g)
{
	if (g) {
		sprites.remove(g);
		g->SetScene(0);
	}
}

// +--------------------------------------------------------------------+

void
SimScene::AddLight(SimLight* l)
{
	if (l) {
		if (!lights.contains(l))
			lights.append(l);

		l->SetScene(this);
	}
}

void
SimScene::DelLight(SimLight* l)
{
	if (l) {
		lights.remove(l);
		l->SetScene(0);
	}
}

// +--------------------------------------------------------------------+

void
SimScene::Collect()
{
	ListIter<Graphic> iter = graphics;

	while (++iter) {
		Graphic* g = iter.value();
		if (g->Life() == 0) {
			delete iter.removeItem();
		}
	}

	iter.attach(sprites);

	while (++iter) {
		Graphic* g = iter.value();
		if (g->Life() == 0) {
			delete iter.removeItem();
		}
	}

	ListIter<SimLight> iter1 = lights;

	while (++iter1) {
		SimLight* l = iter1.value();
		if (l->Life() == 0) {
			delete iter1.removeItem();
		}
	}
}

// +--------------------------------------------------------------------+

bool
SimScene::IsLightObscured(const FVector& obj_pos,
	const FVector& light_pos,
	double         obj_radius,
	FVector* impact_point) const
{
	FVector dir = light_pos - obj_pos;
	const double len = dir.Normalize();

	SimScene* pThis = (SimScene*)this; // cast-away const
	Graphic* g = 0;
	bool      obscured = false;

	ListIter<Graphic> g_iter = pThis->graphics;
	while (++g_iter && !obscured) {
		g = g_iter.value();

		if (g->CastsShadow() && !g->Hidden() && !g->IsInfinite()) {
			const double gdist = (g->Location() - obj_pos).Size();

			if (gdist > 0.1 &&                      // different than object being obscured
				g->Radius() > obj_radius &&         // larger than object being obscured
				(g->Radius() * 400) / gdist > 10) { // projects to a reasonable size

				const FVector delta = (g->Location() - light_pos);

				if (delta.Size() > g->Radius() / 100) { // notxthe object that is emitting the light
					FVector impact = FVector::ZeroVector;
					obscured = g->CheckRayIntersection(obj_pos, dir, len, impact, false) ? true : false;

					if (impact_point)
						*impact_point = impact;
				}
			}

			else if (obj_radius < 0 && gdist < 0.1) { // special case for camera (needed for cockpits)
				const FVector delta = (g->Location() - light_pos);

				if (delta.Size() > g->Radius() / 100) { // notxthe object that is emitting the light
					FVector impact = FVector::ZeroVector;
					obscured = g->CheckRayIntersection(obj_pos, dir, len, impact, false) ? true : false;
				}
			}
		}
	}

	g_iter.attach(pThis->foreground);
	while (++g_iter && !obscured) {
		g = g_iter.value();

		if (g->CastsShadow() && !g->Hidden()) {
			const double gdist = (g->Location() - obj_pos).Size();

			if (gdist > 0.1 &&                      // different than object being obscured
				g->Radius() > obj_radius &&         // larger than object being obscured
				(g->Radius() * 400) / gdist > 10) { // projects to a reasonable size

				const FVector delta = (g->Location() - light_pos);

				if (delta.Size() > g->Radius() / 100) { // notxthe object that is emitting the light
					FVector impact = FVector::ZeroVector;
					obscured = g->CheckRayIntersection(obj_pos, dir, len, impact, false) ? true : false;

					if (impact_point)
						*impact_point = impact;
				}
			}

			else if (obj_radius < 0 && gdist < 0.1) { // special case for camera (needed for cockpits)
				const FVector delta = (g->Location() - light_pos);

				if (delta.Size() > g->Radius() / 100) { // notxthe object that is emitting the light
					FVector impact = FVector::ZeroVector;
					obscured = g->CheckRayIntersection(obj_pos, dir, len, impact, false) ? true : false;
				}
			}
		}
	}

	return obscured;
}