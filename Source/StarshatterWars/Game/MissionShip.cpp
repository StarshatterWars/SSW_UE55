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


#include "MissionShip.h"


MissionShip::MissionShip()
{
	loc.x = -1e9;
	loc.y = -1e9;
	loc.z = -1e9;

	respawns = 0;
	heading = 0;
	integrity = 100;
	decoys = -10;
	probes = -10;
	//skin = 0;
	for (int i = 0; i < 16; i++)
		ammo[i] = -10;

	for (int i = 0; i < 4; i++)
		fuel[i] = -10;
}

MissionShip::~MissionShip()
{
}

// +====================================================================+
	

void
MissionShip::SetAmmo(const int* a)
{
	if (a) {
		for (int i = 0; i < 16; i++)
			ammo[i] = a[i];
	}
}

void
MissionShip::SetFuel(const int* f)
{
	if (f) {
		for (int i = 0; i < 4; i++)
			fuel[i] = f[i];
	}
}

