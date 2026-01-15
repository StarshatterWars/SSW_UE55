/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         CombatAssignment.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	High level assignment of one group to damage another
*/

#pragma once

#include "CoreMinimal.h"
#include "Types.h"
#include "List.h"

// +--------------------------------------------------------------------+

class CombatGroup;
class SimRegion;

// +--------------------------------------------------------------------+

/**
 * 
 */
class STARSHATTERWARS_API CombatAssignment
{
public:
	static const char* TYPENAME() { return "CombatAssignment"; }
	
	CombatAssignment();
	~CombatAssignment();

	CombatAssignment(int t, CombatGroup* obj, CombatGroup* rsc = 0);

	int operator < (const CombatAssignment& a) const;

	// operations:
	void                 SetObjective(CombatGroup* o) { objective = o; }
	void                 SetResource(CombatGroup* r) { resource = r; }

	// accessors:
	int                  Type() { return type; }
	CombatGroup* GetObjective() { return objective; }
	CombatGroup* GetResource() { return resource; }

	const char* GetDescription()  const;
	bool                 IsActive()  const { return resource != 0; }

private:
	int                  type;
	CombatGroup* objective;
	CombatGroup* resource;
};
