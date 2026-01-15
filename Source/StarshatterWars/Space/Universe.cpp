/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Space
	FILE:         Universe.cpp
	AUTHOR:       Carlos Bott
*/


#include "Universe.h"
#include "Galaxy.h"
#include "Campaign.h"

// Sets default values
UUniverse::UUniverse()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//SpawnGalaxy();

	UE_LOG(LogTemp, Log, TEXT("Universe Created"));
}


void UUniverse::Tick(float DeltaTime)
{

}

bool UUniverse::IsTickable() const
{
	return false;
}

bool UUniverse::IsTickableInEditor() const
{
	return false;
}

bool UUniverse::IsTickableWhenPaused() const
{
	return false;
}

TStatId UUniverse::GetStatId() const
{
	return TStatId();
}

UWorld* UUniverse::GetWorld() const
{
	return GetOuter()->GetWorld();
}


void UUniverse::SpawnGalaxy()
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


