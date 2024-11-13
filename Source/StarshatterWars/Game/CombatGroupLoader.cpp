/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         CombatGroupLoader.cpp
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Loader and Parser class for initial generation of Combat Group Data Table
	Will not be used after Dable Table is Generated.
*/


#include "CombatGroupLoader.h"


// Sets default values
ACombatGroupLoader::ACombatGroupLoader()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACombatGroupLoader::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACombatGroupLoader::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

