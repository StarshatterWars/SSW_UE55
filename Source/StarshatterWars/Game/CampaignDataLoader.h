/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         CampaignDataLoader.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Campaign Game Data Loader
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CampaignDataLoader.generated.h"

UCLASS()
class STARSHATTERWARS_API ACampaignDataLoader : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACampaignDataLoader();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
