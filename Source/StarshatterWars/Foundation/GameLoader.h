/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Space
	FILE:         GameLoader.h
	AUTHOR:       Carlos Bott
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/LevelScriptActor.h"
#include "../System/SSWGameInstance.h"
#include "../Foundation/DataLoader.h"
#include "GameLoader.generated.h"

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API AGameLoader : public ALevelScriptActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
public:
	// Sets default values for this actor's properties
	AGameLoader();

	UFUNCTION(BlueprintCallable, Category = "Game Loader")
	USSWGameInstance* GetSSWGameInstance();

	UFUNCTION(BlueprintCallable, Category = "Game Loader")
	void LoadGalaxy();
	void GetGameData();
	void InitializeGame();

	void LoadMainMenu();

	static DataLoader* loader;

	UPROPERTY(EditAnywhere, Category = "UI Sound")
	USoundBase* MenuMusic;
};
