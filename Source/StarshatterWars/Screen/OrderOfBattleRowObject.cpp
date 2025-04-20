// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OrderOfBattleRowObject.h"
#include "../Game/GameStructs.h"  // Adjust the include based on your project

UOrderOfBattleRowObject::UOrderOfBattleRowObject()
{
	// Set default values
	Name = TEXT("Unknown");
	Type = TEXT("None");
	Id = 0;
}

void UOrderOfBattleRowObject::InitializeRow(const FString& InDisplayName, const FString& InType, int32 InId, FS_CombatGroup InCombatGroupData)
{
	Name = InDisplayName;
	Type = InType;
	Id = InId;
	CombatGroupData = InCombatGroupData;
}



