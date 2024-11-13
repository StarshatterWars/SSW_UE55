/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         CombatGroupLoader.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Loader and Parser class for initial generation of Combat Group Data Table 
	Will not be used after Dable Table is Generated.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Foundation/DataLoader.h"
#include "../Foundation/ParseUtil.h"
#include "../Foundation/Random.h"
#include "../Foundation/FormatUtil.h"
#include "../Foundation/Text.h"
#include "../Foundation/Term.h"
#include "../Foundation/GameLoader.h"

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/DataTable.h"

#include "../System/SSWGameInstance.h"
#include "CombatGroupLoader.generated.h"

UCLASS()
class STARSHATTERWARS_API ACombatGroupLoader : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACombatGroupLoader();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	class UDataTable* CombatGroupDataTable;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void LoadCombatGroups();
	void GetSSWInstance();

	FS_CombatGroup FS_CombatGroupUnit;

protected:
	USSWGameInstance* SSWInstance;
};
