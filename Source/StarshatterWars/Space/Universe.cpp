/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Space
	FILE:         Universe.cpp
	AUTHOR:       Carlos Bott
*/


#include "Universe.h"
#include "Galaxy.h"

// Sets default values
AUniverse::AUniverse()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}


// Called when the game starts or when spawned
void AUniverse::BeginPlay()
{
	Super::BeginPlay();
	//SpawnGalaxy();
}

// Called every frame
void AUniverse::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);	
}

void AUniverse::SpawnGalaxy()
{
	UWorld* World = GetWorld();

	FVector location = FVector::ZeroVector;
	FRotator rotate = FRotator::ZeroRotator;

	FActorSpawnParameters SpawnInfo;

	//if (Galaxy == nullptr) {
		AGalaxy* Galaxy = GetWorld()->SpawnActor<AGalaxy>(AGalaxy::StaticClass(), location, rotate, SpawnInfo);


		if (Galaxy)
		{
			UE_LOG(LogTemp, Log, TEXT("Game Galaxy Spawned"));
		}
		else {
			UE_LOG(LogTemp, Log, TEXT("Failed to Spawn Game Galaxy"));
		}

	//else {
	//	UE_LOG(LogTemp, Log, TEXT("Game Universe already exists"));
	//}

	//} else {
	//	UE_LOG(LogTemp, Log, TEXT("World not found"));
	//}		
}


