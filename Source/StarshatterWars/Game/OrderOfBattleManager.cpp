// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OrderOfBattleManager.h"

void UOrderOfBattleManager::Initialize(UDataTable* InCombatGroupTable)
{
	CombatGroupTable = InCombatGroupTable;
	GroupMap.Empty();
	GroupChildren.Empty();
	RootGroups.Empty();

	if (!CombatGroupTable) return;

	TArray<FS_CombatGroup*> AllRows;
	CombatGroupTable->GetAllRows<FS_CombatGroup>(TEXT("OrderOfBattleLoad"), AllRows);

	for (FS_CombatGroup* Row : AllRows)
	{
		if (!Row) continue;

		TSharedPtr<FS_CombatGroup> SharedGroup = MakeShared<FS_CombatGroup>(*Row);
		GroupMap.Add(SharedGroup->Id, SharedGroup);
	}

	BuildHierarchy();
}

void UOrderOfBattleManager::BuildHierarchy()
{
	for (const TPair<int32, TSharedPtr<FS_CombatGroup>>& Pair : GroupMap)
	{
		TSharedPtr<FS_CombatGroup> Group = Pair.Value;
		if (!Group.IsValid()) continue;

		if (Group->ParentId == 0)
		{
			RootGroups.Add(Group);
		}
		else
		{
			GroupChildren.FindOrAdd(Group->ParentId).Add(Group);
		}
	}
}

const TArray<TSharedPtr<FS_CombatGroup>>& UOrderOfBattleManager::GetRootGroups() const
{
	return RootGroups;
}

TArray<TSharedPtr<FS_CombatGroup>> UOrderOfBattleManager::GetChildrenOfGroup(int32 ParentId) const
{
	if (const TArray<TSharedPtr<FS_CombatGroup>>* Found = GroupChildren.Find(ParentId))
	{
		return *Found;
	}
	return {};
}

bool UOrderOfBattleManager::GetGroupById(int32 Id, FS_CombatGroup& OutGroup) const
{
	if (const TSharedPtr<FS_CombatGroup>* Found = GroupMap.Find(Id))
	{
		if (Found && Found->IsValid())
		{
			OutGroup = *(*Found);
			return true;
		}
	}
	return false;
}