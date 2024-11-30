// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameStructs.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FS_ComponentDesign : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Abrv;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int RepairTime;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ReplaceTime;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Spares;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Affects;

	FS_ComponentDesign() {
		Name = "";
		Abrv = "";
		RepairTime = 0;
		ReplaceTime = 0,
			Spares = 1;
		Affects = 0;
	}
};

USTRUCT(BlueprintType)
struct FS_SystemDesign : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_ComponentDesign> Component;

	FS_SystemDesign() {
		Name = "";
	}
};
