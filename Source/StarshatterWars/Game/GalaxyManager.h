// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "../Game/GameStructs.h" // FS_Galaxy struct
#include "../System/SSWGameInstance.h"
#include "GalaxyManager.generated.h"

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UGalaxyManager : public UObject
{
	GENERATED_BODY()
	
	public:
	static UGalaxyManager* Get(UObject* WorldContext);

	void LoadGalaxy(const TArray<FS_Galaxy>& ParsedSystems);
	const FS_Galaxy* FindSystemByName(const FString& Name) const;

	const TArray<FS_Galaxy>& GetAllSystems() const { return Systems; }

private:
	UPROPERTY()
	TArray<FS_Galaxy> Systems;
};

