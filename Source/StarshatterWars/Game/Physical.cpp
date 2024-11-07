// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "Physical.h"


// Sets default values
APhysical::APhysical()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APhysical::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APhysical::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

