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
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Order of Battle")
	FString DisplayName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Order of Battle")
	TArray<UOrderOfBattleRowObject*> Children;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Order of Battle")
	bool bIsUnit;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Order of Battle")
	int32 EntryId;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Order of Battle")
	FS_CombatGroup SourceGroup;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Order of Battle")
	FS_CombatGroupUnit SourceUnit;

	static UOrderOfBattleRowObject* CreateFromGroup(UObject* Outer, const FS_CombatGroup& Group)
	{
		UOrderOfBattleRowObject* NewObj = NewObject<UOrderOfBattleRowObject>(Outer);
		NewObj->SourceGroup = Group;
		NewObj->DisplayName = Group.Name;
		NewObj->bIsUnit = false;
		return NewObj;
	}

	static UOrderOfBattleRowObject* CreateFromUnit(UObject* Outer, const FS_CombatGroupUnit& Unit)
	{
		UOrderOfBattleRowObject* NewObj = NewObject<UOrderOfBattleRowObject>(Outer);
		NewObj->SourceUnit = Unit;
		NewObj->DisplayName = Unit.UnitName;
		NewObj->bIsUnit = true;
		return NewObj;
	}
};
	
	
	

