// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameStructs.h"
#include "../System/SSWGameInstance.h"
#include "../Screen/OrderOfBattleRowObject.h"
#include "../Screen/OrderOfBattleListEntry.h"
#include "OrderOfBattleManager.generated.h"


class UCombatGroupObject;
/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UOrderOfBattleManager : public UObject
{
	GENERATED_BODY()

public:
	// Load data from a DataTable at runtime or in editor
	UFUNCTION(BlueprintCallable, Category = "OrderOfBattle")
	void InitializeFromDataTable(UDataTable* DataTable);

	// Return flat list of combat groups
	UFUNCTION(BlueprintCallable, Category = "OrderOfBattle")
	const TArray<UCombatGroupObject*>& GetCombatGroups() const;

protected:
	// All loaded combat group objects
	UPROPERTY()
	TArray<UCombatGroupObject*> CombatGroupObjects;
};