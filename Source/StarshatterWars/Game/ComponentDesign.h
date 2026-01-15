/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         ComponentDesign.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Generic ship system sub-component class
*/

#pragma once

#include "CoreMinimal.h"
//#include "UObject/NoExportTypes.h"
#include "Types.h"
#include "Geometry.h"
#include "Text.h"

/**
 * 
 */

class STARSHATTERWARS_API ComponentDesign //: public UObject
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
