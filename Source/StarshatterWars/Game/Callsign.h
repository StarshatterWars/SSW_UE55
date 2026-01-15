/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         Callsign.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Package Callsign catalog class
*/

#pragma once

#include "CoreMinimal.h"
#include "Types.h"

/**
 * 
 */
class STARSHATTERWARS_API Callsign
{
public:
	static const char* GetCallsign(int IFF = 1);
};
