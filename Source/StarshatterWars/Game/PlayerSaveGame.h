/*=============================================================================
    Project:        Starshatter Wars (nGenEx Unreal Port)
    Studio:         Fractal Dev Games
    Copyright:      (C) 2024–2026. All Rights Reserved.

    SUBSYSTEM:      SSW
    FILE:           PlayerSaveGame.h
    AUTHOR:         Carlos Bott

    OVERVIEW
    ========
    UPlayerSaveGame

    USaveGame payload for player profile / progress persistence.

    CONTENTS
    ========
    - SaveVersion: format version for forward-compatible migrations
    - PlayerInfo:  FS_PlayerGameInfo payload (profile + progress)

    NOTES
    =====
    - SaveVersion is stored inside the save file and is used by the
      loading code to detect old formats and migrate if needed.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GameStructs.h"
#include "PlayerSaveGame.generated.h"

UCLASS()
class STARSHATTERWARS_API UPlayerSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UPlayerSaveGame();

public:
    // Save format version (increment when the save schema changes)
    UPROPERTY(VisibleAnywhere, Category = "SaveGame Data")
    int32 SaveVersion = 1;

    // Authoritative player payload
    UPROPERTY(VisibleAnywhere, Category = "SaveGame Data")
    FS_PlayerGameInfo PlayerInfo;
};
