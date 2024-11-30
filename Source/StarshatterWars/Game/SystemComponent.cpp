// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "SystemComponent.h"
#include "../System/Game.h"
#include "Component.h"
#include "SystemDesign.h"

// Sets default values for this component's properties
USystemComponent::USystemComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void USystemComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void USystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

USystemComponent::USystemComponent(CATEGORY t, int s, const char* n, int maxv, double energy, double capacity, double sink_rate)
{

}

USystemComponent::~USystemComponent()
{
	components.destroy();
}

// +--------------------------------------------------------------------+

void
USystemComponent::SetDesign(USystemDesign* d)
{
	if (design) {
		design = 0;
		components.destroy();
	}

	if (d) {
		design = d;

		ListIter<ComponentDesign> cd = design->components;
		while (++cd) {
			Component* comp = new Component(cd.value(), this);
			components.append(comp);
		}
	}
}

// +--------------------------------------------------------------------+

void
USystemComponent::SetPowerLevel(double level)
{
	if (level > 100)
		level = 100;
	else if (level < 0)
		level = 0;

	level /= 100;

	if (power_level != level) {
		// if the system is on emergency override power,
		// do not let the EMCON system use this method
		// to drop it back to normal power:
		if (power_level > 1 && level == 1)
			return;

		power_level = (float)level;

		//NetUtil::SendSysStatus(ship, this);
	}
}

void
USystemComponent::SetOverride(bool over)
{
	bool changed = false;

	if (over && power_level != 1.2f) {
		power_level = 1.2f;
		changed = true;
	}

	else if (!over && power_level > 1) {
		power_level = 1.0f;
		changed = true;
	}

	//if (changed)
		//NetUtil::SendSysStatus(ship, this);
}

void
USystemComponent::SetEMCONPower(int index, int pl)
{
	if (index >= 1 && index <= 3) {
		emcon_power[index - 1] = (BYTE)pl;
	}
}

int
USystemComponent::GetEMCONPower(int index)
{
	if (index >= 1 && index <= 3) {
		return emcon_power[index - 1];
	}

	return 100;
}

void
USystemComponent::DoEMCON(int index)
{
	int e = GetEMCONPower(index);

	if (power_level * 100 > e || emcon != index) {
		if (e == 0) {
			PowerOff();
		}
		else {
			if (emcon != index)
				PowerOn();

			SetPowerLevel(e);
		}
	}

	emcon = index;
}

// +--------------------------------------------------------------------+

void
USystemComponent::ExecFrame(double seconds)
{
	if (seconds < 0.01)
		seconds = 0.01;

	STATUS s = DESTROYED;

	if (availability > 0.99)
		s = NOMINAL;
	else if (availability > crit_level)
		s = DEGRADED;
	else
		s = CRITICAL;

	bool repair = false;

	if (components.size() > 0) {
		ListIter<Component> comp = components;
		while (++comp) {
			if (comp->Status() > Component::NOMINAL) {
				repair = true;
				break;
			}
		}
	}

	if (repair) {
		Repair();
	}

	else {
		if (status != s) {
			status = s;
			//NetUtil::SendSysStatus(ship, this);
		}

		// collateral damage due to unsafe operation:
		if (power_on && power_level > safety) {
			safety_overload += (float)seconds;

			// inflict some damage now:
			if (safety_overload > 60) {
				safety_overload -= (float)(rand() / (1000 * (power_level - safety)));
				ApplyDamage(15);

				//NetUtil::SendSysStatus(ship, this);
			}
		}

		else if (safety_overload > 0) {
			safety_overload -= (float)seconds;
		}
	}
}

void
USystemComponent::Repair()
{
	if (status != MAINT) {
		status = MAINT;
		safety_overload = 0.0f;

		//NetUtil::SendSysStatus(ship, this);
	}
}

// +--------------------------------------------------------------------+

void
USystemComponent::ExecMaintFrame(double seconds)
{
	if (components.size() > 0) {
		ListIter<Component> comp = components;
		while (++comp) {
			if (comp->Status() > Component::NOMINAL) {
				comp->ExecMaintFrame(seconds);
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
USystemComponent::ApplyDamage(double damage)
{
	if (!power_on)
		damage /= 10;

	if (components.size() > 0) {
		int index = rand() % components.size();

		if (damage > 50) {
			damage /= 2;
			components[index]->ApplyDamage(damage);

			index = rand() % components.size();
		}

		components[index]->ApplyDamage(damage);

		if (safety < 0.5)
			SetPowerLevel(50);

		else if (safety < 1.0)
			SetPowerLevel(safety * 100);
	}
	else {
		availability -= (float)(damage / 100.0f);
		if (availability < 0.01) availability = 0.0f;
	}
}

void
USystemComponent::CalcStatus()
{
	if (components.size() > 0) {
		availability = 1.0f;
		safety = 1.0f;
		stability = 1.0f;

		ListIter<Component> comp = components;
		while (++comp) {
			if (comp->DamageEfficiency())
				availability *= comp->Availability() / 100.0f;

			if (comp->DamageSafety())
				safety *= comp->Availability() / 100.0f;

			if (comp->DamageStability())
				stability *= comp->Availability() / 100.0f;

			if (comp->IsJerried()) {
				safety *= 0.95f;
				stability *= 0.95f;
			}
		}

		if (net_avail >= 0 && availability < net_avail)
			availability = net_avail;
	}
}

// +--------------------------------------------------------------------+

void
USystemComponent::Mount(Point loc, float rad, float hull)
{
	mount_rel = loc;
	radius = rad;
	hull_factor = hull;
}

void
USystemComponent::Mount(const USystemComponent& system)
{
	mount_rel = system.mount_rel;
	radius = system.radius;
	hull_factor = system.hull_factor;
}

// +--------------------------------------------------------------------+

void
USystemComponent::Orient(const UPhysical* rep)
{
	/*const Matrix& orientation = rep->Cam().Orientation();
	Point         loc = rep->Location();

	mount_loc = (mount_rel * orientation) + loc;
	*/
}

// +----------------------------------------------------------------------+

double
USystemComponent::GetRequest(double seconds) const
{
	if (!power_on || capacity == energy)
		return 0;

	else
		return power_level * sink_rate * seconds;
}

// +----------------------------------------------------------------------+

void
USystemComponent::Distribute(double delivered_energy, double seconds)
{
	if (UsesWatts()) {
		if (seconds < 0.01)
			seconds = 0.01;

		// convert Joules to Watts:
		energy = (float)(delivered_energy / seconds);
	}

	else if (!Game::Paused()) {
		energy += (float)delivered_energy;

		if (energy > capacity)
			energy = capacity;

		else if (energy < 0)
			energy = 0.0f;
	}
}

// +----------------------------------------------------------------------+

void
USystemComponent::DrainPower(double to_level)
{
	energy = 0.0f;
}

