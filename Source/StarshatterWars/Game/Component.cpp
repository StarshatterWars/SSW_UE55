/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         Component.cpp
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Generic ship system sub-component class
*/



#include "Component.h"
#include "SystemComponent.h"
#include "ComponentDesign.h"
#include "../System/Game.h"


// +----------------------------------------------------------------------+

void UComponent::SetComponent(ComponentDesign* d, USystemComponent* s)
{
	design = d;
	system = s;
	status = NOMINAL;
	availability = 100.0f;
	time_remaining = 0.0f;
	spares = 0;
	jerried = 0;

	if (design)
		spares = design->spares;
}

// +----------------------------------------------------------------------+

void UComponent::SetComponent(const UComponent& c)
{
	design = c.design;
	system = c.system;
	status = c.status;
	availability = c.availability;
	time_remaining = c.time_remaining;
	spares = c.spares;
	jerried = c.jerried;
}

// +--------------------------------------------------------------------+

UComponent::~UComponent()
{ }

// +--------------------------------------------------------------------+

void
UComponent::ExecMaintFrame(double seconds)
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
				if (availability > 100) availability = 100.0f;
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
UComponent::ApplyDamage(double damage)
{
	availability -= (float)damage;
	if (availability < 1) availability = 0.0f;

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
UComponent::Repair()
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
UComponent::Replace()
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
UComponent::Availability() const
{
	if (status > NOMINAL && availability > 50)
		return 50.0f;

	return availability;
}

float
UComponent::TimeRemaining() const
{
	return (float)time_remaining;
}

int
UComponent::SpareCount() const
{
	return spares;
}

bool
UComponent::IsJerried() const
{
	return jerried ? true : false;
}

int
UComponent::NumJerried() const
{
	return jerried;
}

const char* UComponent::Name() const
{
	return design->name;
}

const char* UComponent::Abbreviation() const 
{
	return design->abrv;
}

float UComponent::RepairTime() const 
{
	return design->repair_time;
}

float UComponent::ReplaceTime() const 
{
	return design->replace_time;
}

bool UComponent::DamageEfficiency() const 
{
	return (design->affects & DAMAGE_EFFICIENCY) ? true : false; 
}

bool UComponent::DamageSafety() const 
{
	return (design->affects & DAMAGE_SAFETY) ? true : false; 
}

bool UComponent::DamageStability() const 
{
	return (design->affects & DAMAGE_STABILITY) ? true : false;
}