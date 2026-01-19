/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright (C) 2025-2026. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         SimLight.cpp
	AUTHOR:       Carlos Bott
	ORIGINAL:     John DiCamillo / Destroyer Studios LLC

	OVERVIEW
	========
	Dynamic Light Source
*/

#include "SimLight.h"
#include "SimScene.h"

// Minimal Unreal support (required: FVector conversions + UE_LOG):
#include "Math/Vector.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogStarshatterSimLight, Log, All);

// +--------------------------------------------------------------------+

int SimLight::id_key = 1;

// +--------------------------------------------------------------------+

SimLight::SimLight(float l, float dl, int time)
	: id(id_key++),
	type(LIGHT_POINT),
	loc(FVector::ZeroVector),
	life(time),
	light(l),
	dldt(dl),
	color(255, 255, 255),
	active(true),
	shadow(false),
	scene(0)
{
}

// +--------------------------------------------------------------------+

SimLight::~SimLight()
{
}

// +--------------------------------------------------------------------+

void
SimLight::Update()
{
	if (dldt < 1.0f)
		light *= dldt;

	if (life > 0)
		life--;
}

// +--------------------------------------------------------------------+

void
SimLight::Destroy()
{
	if (scene)
		scene->DelLight(this);

	delete this;
}

// +--------------------------------------------------------------------+

void
SimLight::MoveTo(const FVector& dst)
{
	// if (type != LIGHT_DIRECTIONAL)
	loc = dst;
}

void
SimLight::TranslateBy(const FVector& ref)
{
	if (type != LIGHT_DIRECTIONAL)
		loc = loc - ref;
}
