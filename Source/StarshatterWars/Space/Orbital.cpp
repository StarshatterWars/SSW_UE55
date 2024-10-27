/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Space
	FILE:         Orbital.cpp
	AUTHOR:       Carlos Bott
*/



#include "Orbital.h"


// Sets default values
AOrbital::AOrbital()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

AOrbital::AOrbital(AStarSystem* sys, const char* n, EOrbitalType t, double m, double r, double o, AOrbital* p)
{
}

void AOrbital::Update()
{
}

Point AOrbital::PredictLocation(double delta_t)
{
	return Point();
}

// Called when the game starts or when spawned
void AOrbital::BeginPlay()
{
	
}

// Called every frame
void AOrbital::Tick(float DeltaTime)
{

}

