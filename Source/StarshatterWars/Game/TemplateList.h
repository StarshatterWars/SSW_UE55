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
#include "../Foundation/Text.h"
#include "../Foundation/Geometry.h"
#include "../Foundation/Color.h"
#include "../Foundation/Term.h"
#include "../Foundation/List.h"
#include "../Foundation/DataLoader.h"
#include "../System/SSWGameInstance.h"

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
