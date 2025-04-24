// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "../Game/GameStructs.h" // FS_OOBUnit definition
#include "OOBUnitItem.generated.h"

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UOOBUnitItem : public UObject
{
	GENERATED_BODY()
	
public:
	FS_OOBUnit Data;
};
