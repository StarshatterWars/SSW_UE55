/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         MissionLoad.cpp
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Simulation Universe and Region classes
*/


#include "MissionLoad.h"


MissionLoad::MissionLoad()
{
}

MissionLoad::MissionLoad(int s, const char* n)
{
	ship = s;
	for (int i = 0; i < 16; i++)
		load[i] = -1; // default: no weapon mounted

	if (n)
		name = n;

}

void MissionLoad::Initialize(int s, const char* n)
{
	for (int i = 0; i < 16; i++)
		load[i] = -1; // default: no weapon mounted

	if (n)
		name = n;
}

int MissionLoad::GetShip() const
{
	return ship;
}

void MissionLoad::SetShip(int s)
{
	ship = s;
}

Text MissionLoad::GetName() const
{
	return name;
}

void MissionLoad::SetName(Text n)
{
	name = n;
}

int* MissionLoad::GetStations()
{
	return load;
}

int MissionLoad::GetStation(int index)
{
	if (index >= 0 && index < 16)
		return load[index];

	return 0;
}

void MissionLoad::SetStation(int index, int selection)
{
	if (index >= 0 && index < 16)
		load[index] = selection;
}
