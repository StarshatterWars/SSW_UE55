/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         SystemDesign.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Generic Ship Design class
	Note: Loaded from Data Tables
*/

#pragma once

#include "../Foundation/Types.h"
#include "../Foundation/List.h"
#include "../Foundation/Text.h"
#include "CoreMinimal.h"
#include "GameStructs.h"


// +--------------------------------------------------------------------+

class ComponentDesign;
class SystemDesign;

// +--------------------------------------------------------------------+

/**
 * 
 */
class STARSHATTERWARS_API SystemDesign
{
public:
	SystemDesign();
	virtual ~SystemDesign();

	static const char* TYPENAME() { return "SystemDesign"; }

	int operator == (const SystemDesign& rhs) const { return name == rhs.name; }

	static void          Initialize();
	static void          Close();
	static SystemDesign* Find(const char* name);

	// Unique ID:
	Text              name;

	// Sub-components:
	List<ComponentDesign> components;

	static List<SystemDesign>  catalog;

	TArray<FS_SystemDesign> SystemDesignArray;

	class UDataTable* SystemDesignDataTable;
};
