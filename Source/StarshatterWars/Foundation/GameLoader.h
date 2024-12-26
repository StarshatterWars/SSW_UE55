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
	UFUNCTION(BlueprintCallable, Category = "Game Loader")
	USSWGameInstance* GetSSWGameInstance();

	UFUNCTION(BlueprintCallable, Category = "Game Loader")
	void LoadGalaxy();
	void GetCombatRosterData();
	void GetCampaignData();
	void GetAwardInfoData();
	void InitializeGame();

	void ShowMainMenu();
	void ShowQuitDlg();

	static DataLoader* loader;
};
