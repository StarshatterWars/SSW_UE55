// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "../Game/GameStructs.h"
#include "RosterViewObject.generated.h"

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API URosterViewObject : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "List Item")
	FString GroupName;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "List Item")
	FString GroupType;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "List Item")
	TEnumAsByte<ECOMBATGROUP_TYPE> GroupEType;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "List Item")
	FString GroupLocation;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "List Item")
	int32 GroupId;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "List Item")
	int32 GroupParentId;

};
