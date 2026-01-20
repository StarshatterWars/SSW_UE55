/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright © 2025-2026. All Rights Reserved.

	ORIGINAL AUTHOR AND STUDIO: John DiCamillo / Destroyer Studios LLC

	SUBSYSTEM:    Stars.exe
	FILE:         DropShipAI.cpp
	AUTHOR:       Carlos Bott


	OVERVIEW
	========
	Drop Ship (orbit/surface and surface/orbit) AI class
*/

#include "DropShipAI.h"

#include "Math/Vector.h"
#include "Math/UnrealMathUtility.h"
#include "Logging/LogMacros.h"

#include "TacticalAI.h"
#include "Ship.h"
#include "ShipManager.h"
#include "Drive.h"
#include "Sim.h"
#include "StarSystem.h"
#include "KeyMap.h"

#include "Game.h"

#ifndef STARSHATTERWARS_LOG_DEFINED
#define STARSHATTERWARS_LOG_DEFINED
DECLARE_LOG_CATEGORY_EXTERN(LogStarshatterWars, Log, All);
#endif

// +----------------------------------------------------------------------+

DropShipAI::DropShipAI(Ship* s)
	: ShipAI(s)
{
	seek_gain = 20;
	seek_damp = 0.5;

	delete tactical;
	tactical = 0;
}

DropShipAI::~DropShipAI()
{
}

// +--------------------------------------------------------------------+

void
DropShipAI::FindObjective()
{
	distance = 0;

	if (!ship)  return;

	Sim* sim = Sim::GetSim();
	SimRegion* self_rgn = ship->GetRegion();

	// if making orbit, go up:
	if (self_rgn->Type() == Sim::AIR_SPACE) {
		obj_w = self->Location() + FVector(0.0f, 1.0e3f, 0.0f);
	}

	// if breaking orbit, head for terrain region:
	else {
		SimRegion* dst_rgn = sim->FindNearestTerrainRegion(ship);

		const FVector dst =
			dst_rgn->GetOrbitalRegion()->Location() -
			self_rgn->GetOrbitalRegion()->Location() +
			FVector(0.0f, 0.0f, -1.0e6f);

		obj_w = dst.OtherHand();
	}

	// distance from self to navpt:
	distance = FVector(obj_w - self->Location()).Length();

	// transform into camera coords:
	objective = Transform(obj_w);
	objective.Normalize();
}

// +--------------------------------------------------------------------+

void
DropShipAI::Navigator()
{
	accumulator.Clear();
	magnitude = 0;

	if (other)
		ship->SetFLCSMode(Ship::FLCS_AUTO);
	else
		ship->SetFLCSMode(Ship::FLCS_MANUAL);

	Accumulate(AvoidCollision());
	Accumulate(Seek(objective));

	// are we being asked to flee?
	if (FMath::Abs(accumulator.yaw) == 1.0 && accumulator.pitch == 0.0) {
		accumulator.pitch = -0.7f;
		accumulator.yaw *= 0.25f;
	}

	self->ApplyRoll((float)(accumulator.yaw * -0.4));
	self->ApplyYaw((float)(accumulator.yaw * 0.2));

	if (FMath::Abs(accumulator.yaw) > 0.5 && FMath::Abs(accumulator.pitch) < 0.1)
		accumulator.pitch -= 0.1f;

	if (accumulator.pitch != 0)
		self->ApplyPitch((float)accumulator.pitch);

	// if not turning, roll to orient with world coords:
	if (FMath::Abs(accumulator.yaw) < 0.1) {
		const FVector Vrt = ((Camera*)&(self->Cam()))->vrt();
		const double  Deflection = Vrt.Y;

		if (Deflection != 0) {
			const double Theta = FMath::Asin(Deflection / Vrt.Length());
			self->ApplyRoll(-Theta);
		}
	}

	ship->SetThrottle(100);
	ship->ExecFLCSFrame();
}
