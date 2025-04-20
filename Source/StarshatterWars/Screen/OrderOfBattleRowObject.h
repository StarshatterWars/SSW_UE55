// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "../Game/GameStructs.h" 
#include "OrderOfBattleRowObject.generated.h"

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UOrderOfBattleRowObject : public UObject
{
	GENERATED_BODY()
	
public:
	// Name to display for the row
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OrderOfBattle")
	FString Name;

	// Additional fields as necessary
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OrderOfBattle")
	FString Type;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OrderOfBattle")
	int32 Id;

	// A reference to the combat group or other data to represent this row's context
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OrderOfBattle")
	FS_CombatGroup CombatGroupData;

	// Constructor
	UOrderOfBattleRowObject();

	// Optionally add functions to manipulate data if needed
	void InitializeRow(const FString& InDisplayName, const FString& InType, int32 InId, FS_CombatGroup InCombatGroupData);
};
	
	
	

