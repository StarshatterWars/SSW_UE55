// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "Orbital.h"


// Sets default values
AOrbital::AOrbital()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AOrbital::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AOrbital::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AOrbital::Update()
{
}

Point AOrbital::PredictLocation(double delta_t)
{
	return Point();
}

