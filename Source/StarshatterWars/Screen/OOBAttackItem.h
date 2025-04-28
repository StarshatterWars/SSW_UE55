// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "../Game/GameStructs.h" // FS_OOBAttack definition
#include "OOBAttackItem.generated.h"

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UOOBAttackItem : public UObject
{
	GENERATED_BODY()
	
public:
	FS_OOBAttack Data;	
};
