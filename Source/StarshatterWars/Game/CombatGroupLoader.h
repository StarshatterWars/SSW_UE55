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
#include "HAL/FileManagerGeneric.h"
#include "Misc/Paths.h"
#include "Engine/DataTable.h"
#include "GameStructs.h"

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

	void LoadCombatRoster();

	void LoadOrderOfBattle(const char* filename, int team);
	void GetSSWInstance();

	void ParseCombatUnit();

	FString GetOrdinal(int id);
	FString GetNameFromType(FString name);

	FS_CombatGroupUnit CombatGroupUnit;
	FS_CombatGroup CombatGroupData;
	TArray<FS_CombatGroupUnit> NewCombatUnitArray;

protected:
	USSWGameInstance* SSWInstance;

	Text  Name;
	Text  Type;
	Text  Intel;
	Text  Region;
	Text  System;
	Text  ParentType;
	int   ParentId;
	int   UnitIndex;
	int   Id;
	int   Iff;
	Vec3  Loc;

	Text UnitName;
	Text UnitRegnum;
	Text UnitRegion;
	Text UnitClass;
	Text UnitDesign;
	Text UnitSkin;
	Vec3 UnitLoc;
	int  UnitCount;
	int  UnitDamage;
	int  UnitDead;
	int  UnitHeading;
};
