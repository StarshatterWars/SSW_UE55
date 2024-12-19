/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         AwardInfoLoader.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Loader and Parser class for initial generation of the Award Info Data Table
	Will not be used after Data Table is Generated.
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
#include "../Game/GameStructs.h"


#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/DataTable.h"

#include "../System/SSWGameInstance.h"
#include "AwardInfoLoader.generated.h"

UCLASS()
class STARSHATTERWARS_API AAwardInfoLoader : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAwardInfoLoader();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	class UDataTable* AwardsDataTable;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
		
	void LoadAwardTables();
	void GetSSWInstance();

	FS_AwardInfo AwardData;

protected:
	// attributes:

	USSWGameInstance* SSWInstance;

	int      AwardId;
	int		 AwardGrant;
	char	 filename[64];

	Text     AwardType;
	Text     AwardName;
	Text     AwardAbrv;
	Text     AwardDesc;
	Text     AwardText;

	Text     DescSound;
	Text     GrantSound;

	Text	LargeImage;
	Text	SmallImage;

	int		RequiredAwards;
	int		Lottery;
	int		MinRank;
	int		MaxRank;
	int     MinShipClass;
	int     MaxShipClass;
	int		GrantedShipClasses;

	int		TotalPoints;
	int		MissionPoints;
	int		TotalMissions;

	int		Kills;
	int		Lost;
	int		Collision;
	int		CampaignId;

	bool	CampaignComplete;
	bool	DynamicCampaign;
	bool	Ceremony;

};
