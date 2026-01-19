/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         ShipDataLoader.cpp
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Loader and Parser class for initial generation of Ship Data Table
	Will notxbe used after Dable Table is Generated.
*/

#include "ShipDataLoader.h"


// Sets default values
AShipDataLoader::AShipDataLoader()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AShipDataLoader::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AShipDataLoader::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

