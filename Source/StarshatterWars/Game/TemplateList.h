/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         TemplateList.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Campaign defines a strategic military scenario.
*/

#pragma once

#include "CoreMinimal.h"
#include "Text.h"
#include "Geometry.h"
#include "Color.h"
#include "Term.h"
#include "List.h"
#include "DataLoader.h"
#include "SSWGameInstance.h"

class MissionInfo;
/**
 * 
 */
class STARSHATTERWARS_API TemplateList
{
public:
	static const char* TYPENAME() { return "TemplateList"; }

	TemplateList();
	~TemplateList();

	int               mission_type;
	int               group_type;
	int               index;
	List<MissionInfo> missions;

};
