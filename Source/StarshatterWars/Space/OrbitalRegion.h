// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Orbital.h"
#include "OrbitalRegion.generated.h"

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API AOrbitalRegion : public AOrbital
{
	GENERATED_BODY()
	
public:
	
	static const char* TYPENAME() { return "OrbitalRegion"; }
	
	// Sets default values for this actor's properties
	AOrbitalRegion();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	double            GridSpace()       const { return grid; }
	double            Inclination()     const { return inclination; }
	int               Asteroids()       const { return asteroids; }
	List<Text>& Links() { return links; }

protected:
	double            grid;
	double            inclination;
	int               asteroids;
	List<Text>        links;
	
};
