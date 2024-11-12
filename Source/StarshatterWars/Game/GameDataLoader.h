/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         GameDataLoader.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Master Game Data Loader
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Foundation/DataLoader.h"
#include "../Foundation/ParseUtil.h"
#include "../Foundation/Random.h"
#include "../Foundation/FormatUtil.h"
#include "../Foundation/GameLoader.h"

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/DataTable.h"

#include "../System/SSWGameInstance.h"
#include "GameDataLoader.generated.h"

class Combatant;

UCLASS()
class STARSHATTERWARS_API AGameDataLoader : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGameDataLoader();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void LoadCampaignData(const char* FileName);
	void LoadGalaxyData();

	void GetSSWInstance();

	void InitializeCampaignData();
	USSWGameInstance* SSWInstance;

protected:
	char                 filename[64];
	Text                 name;
	Text                 description;
	Text                 situation;
	Text                 orders;
	bool                 scripted;
	bool                 sequential;

	List<Combatant>      combatants;
};
