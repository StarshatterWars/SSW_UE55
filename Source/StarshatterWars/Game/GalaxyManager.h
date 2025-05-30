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

	UFUNCTION()
	UTextureRenderTarget2D* GetOrCreatePlanetRenderTarget(const FString& PlanetName, int32 Resolution, UObject* Object = nullptr);

	UFUNCTION()
	UTextureRenderTarget2D* GetOrCreateStarRenderTarget(const FString& PlanetName, int32 Resolution, UObject* Object = nullptr);

	UFUNCTION()
	UTextureRenderTarget2D* GetOrCreateMoonRenderTarget(const FString& MoonName, int32 Resolution, UObject* Object = nullptr);
	UFUNCTION()
	void ClearAllRenderTargets();

	UFUNCTION()
	const TArray<FS_Galaxy>& GetAllSystems() const { return Systems; }

	UPROPERTY()
	TMap<FString, UTextureRenderTarget2D*> PlanetRenderTargets;

	UPROPERTY()
	TMap<FString, UTextureRenderTarget2D*> MoonRenderTargets;

	UPROPERTY()
	TMap<FString, UTextureRenderTarget2D*> StarRenderTargets;


private:
	UPROPERTY()
	TArray<FS_Galaxy> Systems;
};

