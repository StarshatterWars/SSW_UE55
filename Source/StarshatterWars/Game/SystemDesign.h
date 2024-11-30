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
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "GameStructs.h"


// +--------------------------------------------------------------------+

class ComponentDesign;

// +--------------------------------------------------------------------+

/**
 * 
 */
class STARSHATTERWARS_API USystemDesign  : public UObject
{
public:
	USystemDesign();
	virtual ~USystemDesign();

	static const char* TYPENAME() { return "SystemDesign"; }

	int operator == (const USystemDesign& rhs) const { return name == rhs.name; }

	static void		     Initialize(UDataTable* SystemDT);
	static void          Close();
	static USystemDesign* Find(const char* name);

	// Unique ID:
	Text              name;

	// Sub-components:
	List<ComponentDesign> components;

	static List<USystemDesign>  catalog;

	TArray<FS_SystemDesign> SystemDesignArray;
};
