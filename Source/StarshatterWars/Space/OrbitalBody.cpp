/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Space
	FILE:         OrbitalBody.cpp
	AUTHOR:       Carlos Bott
*/


#include "OrbitalBody.h"

// Sets default values
AOrbitalBody::AOrbitalBody()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Orbital Body"));
	RootComponent = Root;
}

void AOrbitalBody::Initialize(AStarSystem* sys, FString n, EOrbitalType t, double m, double r, double o, AOrbital* prime)
{
}

// Called when the game starts or when spawned
void AOrbitalBody::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AOrbitalBody::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AOrbitalBody::Update()
{
}

