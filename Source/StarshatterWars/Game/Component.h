/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         Component.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Generic ship system sub-component class
*/

#pragma once

#include "CoreMinimal.h"
#include "../Foundation/Types.h"
#include "../Foundation/Geometry.h"
#include "../Foundation/Text.h"
/**
 * 
 */

class USystemComponent;

class STARSHATTERWARS_API ComponentDesign
{
public:
	static const char* TYPENAME() { return "ComponentDesign"; }

	ComponentDesign();
	virtual ~ComponentDesign();
	int operator == (const ComponentDesign& rhs) const { return (name == rhs.name); }

	// identification:
	Text              name;
	Text              abrv;

	float             repair_time;
	float             replace_time;
	int               spares;
	DWORD             affects;
};
 
class STARSHATTERWARS_API Component
{
public:
	Component();
	virtual ~Component();

	static const char* TYPENAME() { return "Component"; }

	enum STATUS { DESTROYED, CRITICAL, DEGRADED, NOMINAL, REPLACE, REPAIR };
	enum DAMAGE {
		DAMAGE_EFFICIENCY = 0x01,
		DAMAGE_SAFETY = 0x02,
		DAMAGE_STABILITY = 0x04
	};

	Component(ComponentDesign* d, USystemComponent* s);
	Component(const Component& c);

	const char* Name()         const { return design->name; }
	const char* Abbreviation() const { return design->abrv; }
	float             RepairTime()   const { return design->repair_time; }
	float             ReplaceTime()  const { return design->replace_time; }

	bool              DamageEfficiency() const { return (design->affects & DAMAGE_EFFICIENCY) ? true : false; }
	bool              DamageSafety()     const { return (design->affects & DAMAGE_SAFETY) ? true : false; }
	bool              DamageStability()  const { return (design->affects & DAMAGE_STABILITY) ? true : false; }

	STATUS            Status()       const { return status; }
	float             Availability() const;
	float             TimeRemaining() const;
	int               SpareCount()   const;
	bool              IsJerried()    const;
	int               NumJerried()   const;

	void              SetSystem(USystemComponent* s) { system = s; }
	USystemComponent* GetSystem()    const { return system; }

	virtual void      ApplyDamage(double damage);
	virtual void      ExecMaintFrame(double seconds);
	virtual void      Repair();
	virtual void      Replace();

protected:
	ComponentDesign* design;

	// Component health status:
	STATUS            status;
	float             availability;
	float             time_remaining;
	int               spares;
	int               jerried;
	USystemComponent* system;
};
