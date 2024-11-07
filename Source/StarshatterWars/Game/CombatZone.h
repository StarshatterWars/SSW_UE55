/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         CombatZone.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	CombatZone is used by the dynamic campaign strategy
	and logistics algorithms to assign forces to locations
	within the campaign.  A CombatZone is a collection of
	closely related sectors, and the assets contained
	within them.
*/


#pragma once

#include "CoreMinimal.h"
#include "../Foundation/Types.h"
#include "../Foundation/Geometry.h"
#include "../Foundation/Text.h"
#include "../Foundation/List.h"

// +--------------------------------------------------------------------+

class CombatGroup;
class CombatUnit;
class ZoneForce;

// +--------------------------------------------------------------------+

/**
 * 
 */
class STARSHATTERWARS_API CombatZone
{
public:
	static const char* TYPENAME() { return "CombatZone"; }

	CombatZone();
	~CombatZone();

	int operator == (const CombatZone& g)  const { return this == &g; }

	const Text& Name()            const { return name; }
	const Text& System()          const { return system; }
	void                 AddGroup(CombatGroup* g);
	void                 RemoveGroup(CombatGroup* g);
	bool                 HasGroup(CombatGroup* g);
	void                 AddRegion(const char* rgn);
	bool                 HasRegion(const char* rgn);
	List<Text>& GetRegions() { return regions; }
	List<ZoneForce>& GetForces() { return forces; }

	ZoneForce* FindForce(int iff);
	ZoneForce* MakeForce(int iff);

	void                 Clear();

	static List<CombatZone>&
		Load(const char* filename);

private:
	// attributes:
	Text                 name;
	Text                 system;
	List<Text>           regions;
	List<ZoneForce>      forces;
};
