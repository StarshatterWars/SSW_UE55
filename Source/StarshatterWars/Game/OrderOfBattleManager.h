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
	UFUNCTION(BlueprintCallable, Category = "Order of Battle")
	void Initialize(UDataTable* InCombatGroupTable);

	const TArray<TSharedPtr<FS_CombatGroup>>& GetRootGroups() const;
	TArray<TSharedPtr<FS_CombatGroup>> GetChildrenOfGroup(int32 ParentId) const;

	UFUNCTION(BlueprintCallable, Category = "Order of Battle")
	bool GetGroupById(int32 Id, FS_CombatGroup& OutGroup) const;

protected:
	UPROPERTY()
	UDataTable* CombatGroupTable;

	TMap<int32, TSharedPtr<FS_CombatGroup>> GroupMap;
	TMap<int32, TArray<TSharedPtr<FS_CombatGroup>>> GroupChildren;
	TArray<TSharedPtr<FS_CombatGroup>> RootGroups;

	void BuildHierarchy();
};