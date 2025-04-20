// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameStructs.h"
#include "../System/SSWGameInstance.h"
#include "Templates/SharedPointer.h"
#include "OrderOfBattleManager.generated.h"

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UOrderOfBattleManager : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UDataTable* CombatGroupTable);

	const FS_CombatGroup* GetGroupById(int32 GroupId) const;

	TArray<int32> GetRootGroupIds() const;
	TArray<int32> GetChildrenOfGroup(int32 ParentId) const;

protected:
	UPROPERTY()
	TMap<int32, FS_CombatGroup> AllGroups;
	TMap<int32, TArray<int32>> GroupChildrenMap;
};