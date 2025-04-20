// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OrderOfBattleManager.h"

void UOrderOfBattleManager::Initialize(UDataTable* CombatGroupTable)
{
	AllGroups.Empty();
	GroupChildrenMap.Empty();

	if (!CombatGroupTable) return;

	TArray<FName> RowNames = CombatGroupTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		FS_CombatGroup* Row = CombatGroupTable->FindRow<FS_CombatGroup>(RowName, TEXT("Init OrderOfBattle"));
		if (Row)
		{
			AllGroups.Add(Row->Id, *Row);

			// Build child map
			GroupChildrenMap.FindOrAdd(Row->ParentId).Add(Row->Id);
		}
	}
}

const FS_CombatGroup* UOrderOfBattleManager::GetGroupById(int32 GroupId) const
{
	return AllGroups.Find(GroupId);
}

TArray<int32> UOrderOfBattleManager::GetRootGroupIds() const
{
	TArray<int32> RootGroups;

	for (const auto& Pair : AllGroups)
	{
		if (!AllGroups.Contains(Pair.Value.ParentId))
		{
			RootGroups.Add(Pair.Key);
		}
	}

	return RootGroups;
}

TArray<int32> UOrderOfBattleManager::GetChildrenOfGroup(int32 ParentId) const
{
	const TArray<int32>* Found = GroupChildrenMap.Find(ParentId);
	return Found ? *Found : TArray<int32>();
}