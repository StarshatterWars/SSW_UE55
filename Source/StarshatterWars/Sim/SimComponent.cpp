/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright (C) 2025-2026. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         SimComponent.cpp
	AUTHOR:       Carlos Bott

	ORIGINAL AUTHOR AND STUDIO
	==========================
	John DiCamillo / Destroyer Studios LLC

	OVERVIEW
	========
	Generic ship system sub-component class
*/

#include "SimComponent.h"
#include "SimSystem.h"
#include "Game.h"

// +----------------------------------------------------------------------+

ComponentDesign::ComponentDesign()
	: repair_time(0.0f),
	replace_time(0.0f),
	spares(0),
	affects(0)
{
}

// +----------------------------------------------------------------------+

ComponentDesign::~ComponentDesign()
{
}

// +----------------------------------------------------------------------+

SimComponent::SimComponent(ComponentDesign* d, SimSystem* s)
	: design(d),
	status(NOMINAL),
	availability(100.0f),
	time_remaining(0.0f),
	spares(0),
	jerried(0),
	system(s)
{
	if (design)
		spares = design->spares;
}

// +----------------------------------------------------------------------+

SimComponent::SimComponent(const SimComponent& c)
	: design(c.design),
	status(c.status),
	availability(c.availability),
	time_remaining(c.time_remaining),
	spares(c.spares),
	jerried(c.jerried),
	system(c.system)
{
}

// +--------------------------------------------------------------------+

SimComponent::~SimComponent()
{
}

// +--------------------------------------------------------------------+

void
SimComponent::ExecMaintFrame(double seconds)
{
	if (status > NOMINAL) {
		time_remaining -= (float)seconds;

		// when repairs are complete:
		if (time_remaining <= 0) {
			if (status == REPAIR) {
				// did we just jerry-rig a failed component?
				if (availability < 50)
					jerried++;

				if (jerried < 5)
					availability += 50.0f - 10 * jerried;
				if (availability > 100)
					availability = 100.0f;
			}
			else {
				availability = 100.0f;
			}

			if (availability > 99)
				status = NOMINAL;
			else if (availability > 49)
				status = DEGRADED;
			else
				status = CRITICAL;

			time_remaining = 0.0f;

			if (system)
				system->CalcStatus();
		}
	}
}

// +--------------------------------------------------------------------+

void
SimComponent::ApplyDamage(double damage)
{
	availability -= (float)damage;
	if (availability < 1)
		availability = 0.0f;

	if (status < REPLACE) {
		if (availability > 99)
			status = NOMINAL;
		else if (availability > 49)
			status = DEGRADED;
		else
			status = CRITICAL;
	}

	if (system)
		system->CalcStatus();
}

// +--------------------------------------------------------------------+

void
SimComponent::Repair()
{
	if (status < NOMINAL) {
		status = REPAIR;
		time_remaining = design->repair_time;

		if (system)
			system->CalcStatus();
	}
}

// +--------------------------------------------------------------------+

void
SimComponent::Replace()
{
	if (status <= NOMINAL) {
		status = REPLACE;
		spares--;
		time_remaining = design->replace_time;

		if (system)
			system->CalcStatus();
	}
}

// +--------------------------------------------------------------------+

float
SimComponent::Availability() const
{
	if (status > NOMINAL && availability > 50)
		return 50.0f;

	return availability;
}

float
SimComponent::TimeRemaining() const
{
	return (float)time_remaining;
}

int
SimComponent::SpareCount() const
{
	return spares;
}

bool
SimComponent::IsJerried() const
{
	return jerried ? true : false;
}

int
SimComponent::NumJerried() const
{
	return jerried;
}
