/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         ShipDataLoader.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Loader and Parser class for initial generation of Ship Data Table
	Will not be used after Dable Table is Generated.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShipDataLoader.generated.h"

UCLASS()
class STARSHATTERWARS_API AShipDataLoader : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AShipDataLoader();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
