// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "../Game/GameStructs.h"
#include "CombatGroupObject.generated.h"

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UCombatGroupObject : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Combat Group")
	FS_CombatGroup Data;
};
	
	
	

