// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OrderOfBattleManager.h"
#include "../Screen/OrderOfBattleRowObject.h"
#include "Engine/Engine.h"

void UOrderOfBattleManager::InitializeFromDataTable(UDataTable* DataTable)
{
	CombatGroupObjects.Empty();

	if (!DataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("OrderOfBattleManager: DataTable is null"));
		return;
	}

	TArray<FS_CombatGroup*> AllRows;
	DataTable->GetAllRows<FS_CombatGroup>(TEXT("OrderOfBattle Init"), AllRows);

	for (FS_CombatGroup* Row : AllRows)
	{
		if (!Row) continue;

		UCombatGroupObject* GroupObj = NewObject<UCombatGroupObject>(this);
		GroupObj->Data = *Row;

		CombatGroupObjects.Add(GroupObj);
	}

	UE_LOG(LogTemp, Log, TEXT("OrderOfBattleManager: Loaded %d combat groups."), CombatGroupObjects.Num());
}

const TArray<UCombatGroupObject*>& UOrderOfBattleManager::GetCombatGroups() const
{
	return CombatGroupObjects;
}