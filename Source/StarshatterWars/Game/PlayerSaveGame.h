// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GameStructs.h"
#include "PlayerSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UPlayerSaveGame : public USaveGame
{
	GENERATED_BODY()
	
	UPlayerSaveGame();
	
public:
    UPROPERTY(VisibleAnywhere, Category = "SaveGame Data")
    FS_PlayerGameInfo PlayerInfo;

};
