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
#include "Types.h"
#include "Geometry.h"
#include "Text.h"
#include "UObject/NoExportTypes.h"

/**
 * 
 */

class USystemComponent;
class ComponentDesign;

class STARSHATTERWARS_API UComponent : public UObject
{
public:

	UComponent();
	virtual ~UComponent();

	static const char* TYPENAME() { return "Component"; }

	enum STATUS { DESTROYED, CRITICAL, DEGRADED, NOMINAL, REPLACE, REPAIR };
	enum DAMAGE {
		DAMAGE_EFFICIENCY = 0x01,
		DAMAGE_SAFETY = 0x02,
		DAMAGE_STABILITY = 0x04
	};

	void SetComponent(ComponentDesign* d, USystemComponent* s);
	void SetComponent(const UComponent& c);

	UComponent(ComponentDesign* d, USystemComponent* s);
	UComponent(const UComponent& c);

	const char*	 	  Name() const;
	const char*		  Abbreviation() const;
	float             RepairTime() const;
	float             ReplaceTime() const;

	bool              DamageEfficiency() const;
	bool              DamageSafety() const;
	bool              DamageStability() const;

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
