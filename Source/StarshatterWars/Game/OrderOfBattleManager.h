// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameStructs.h"
#include "../System/SSWGameInstance.h"
#include "../Screen/OrderOfBattleRowObject.h"
#include "../Screen/OrderOfBattleListEntry.h"
#include "OrderOfBattleManager.generated.h"

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UOrderOfBattleManager : public UObject
{
	GENERATED_BODY()

public:
	// Store a collection of row objects (these would represent each order of battle item)
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Order Of Battle")
	TArray<UOrderOfBattleRowObject*> OrderOfBattleItems;

	// Load data into the manager (could be from file or hardcoded)
	void LoadOrderOfBattleData();

	// Add a new entry to the order of battle
	void AddEntry(const UOrderOfBattleListEntry& Entry);

	// Get all order of battle items
	const TArray<UOrderOfBattleRowObject*>& GetOrderOfBattleItems() const;

	// Search for a specific entry by ID or name (example)
	UOrderOfBattleRowObject* GetOrderOfBattleEntryById(int32 Id);

protected:
	virtual void InitializeOrderOfBattleData();
};