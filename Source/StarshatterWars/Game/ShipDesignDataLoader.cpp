/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         ShipDesignDataLoader.cpp
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Loader and Parser class for initial generation of Ship Design Data Table
	Will not be used after Dable Table is Generated.
*/


#include "ShipDesignDataLoader.h"


// Sets default values
AShipDesignDataLoader::AShipDesignDataLoader()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AShipDesignDataLoader::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AShipDesignDataLoader::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

