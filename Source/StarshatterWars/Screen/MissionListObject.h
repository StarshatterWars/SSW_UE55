// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MissionListObject.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class STARSHATTERWARS_API UMissionListObject : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "List Item")
	FString MissionName;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "List Item")
	FString MissionSitrep;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "List Item")
	FString MissionDesc;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "List Item")
	FString MissionRegion;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "List Item")
	FString MissionSystem;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "List Item")
	FString MissionObjective;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "List Item")
	FString MissionStatus;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "List Item")
	FString MissionTime;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "List Item")
	FString MissionType;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "List Item")
	int32 MissionId;
	
	
	
};
