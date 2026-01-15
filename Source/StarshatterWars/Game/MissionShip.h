/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         MissionShip.cpp
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Simulation Universe and Region classes
*/

#pragma once

#include "CoreMinimal.h"
#include "Types.h"
#include "Text.h"
#include "Geometry.h"

/**
 * 
 */
class STARSHATTERWARS_API MissionShip
{
public:
	
	friend class Mission;

	static const char* TYPENAME() { return "MissionShip"; }

	MissionShip();
	~MissionShip(); 

	const Text Name()                  const { return name; }
	const Text RegNum()                const { return regnum; }
	const Text Region()                const { return region; }
	//const Skin* GetSkin()               const { return skin; }
	const Point& Location()              const { return loc; }
	const Point& Velocity()              const { return velocity; }
	int               Respawns()              const { return respawns; }
	double            Heading()               const { return heading; }
	double            Integrity()             const { return integrity; }
	int               Decoys()                const { return decoys; }
	int               Probes()                const { return probes; }
	const int* Ammo()                  const { return ammo; }
	const int* Fuel()                  const { return fuel; }

	void              SetName(const char* n) { name = n; }
	void              SetRegNum(const char* n) { regnum = n; }
	void              SetRegion(const char* n) { region = n; }
	//void              SetSkin(const Skin* s) { skin = s; }
	void              SetLocation(const Point& p) { loc = p; }
	void              SetVelocity(const Point& p) { velocity = p; }
	void              SetRespawns(int r) { respawns = r; }
	void              SetHeading(double h) { heading = h; }
	void              SetIntegrity(double n) { integrity = n; }
	void              SetDecoys(int d) { decoys = d; }
	void              SetProbes(int p) { probes = p; }
	void              SetAmmo(const int* a);
	void              SetFuel(const int* f);

protected:
	Text              name;
	Text              regnum;
	Text              region;
	//const Skin* skin;
	Point             loc;
	Point             velocity;
	int               respawns;
	double            heading;
	double            integrity;
	int               decoys;
	int               probes;
	int               ammo[16];
	int               fuel[4];
};
