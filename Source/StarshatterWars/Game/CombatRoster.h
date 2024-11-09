/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         CombatRoster.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	The complete roster of all known persistent entities
	for all combatants in the game.
*/

#pragma once

#include "CoreMinimal.h"
#include "../Foundation/Types.h"
#include "../Foundation/Geometry.h"
#include "../Foundation/List.h"

// +--------------------------------------------------------------------+

class CombatGroup;

// +--------------------------------------------------------------------+

/**
 * 
 */
class STARSHATTERWARS_API CombatRoster
{
public:
	static const char* TYPENAME() { return "CombatRoster"; }
	
	CombatRoster();
	~CombatRoster();

	static void             Initialize();
	static void             Close();
	static CombatRoster* GetInstance();

	CombatGroup* GetForce(const char* name);

private:
	List<CombatGroup>       forces;
};
