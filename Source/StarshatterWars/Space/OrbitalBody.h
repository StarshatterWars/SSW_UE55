/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Space
	FILE:         OrbitalBody.h
	AUTHOR:       Carlos Bott
*/

#pragma once

#include "CoreMinimal.h"
#include "Orbital.h"
#include "OrbitalBody.generated.h"

UCLASS()
class STARSHATTERWARS_API AOrbitalBody : public AOrbital
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOrbitalBody();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
