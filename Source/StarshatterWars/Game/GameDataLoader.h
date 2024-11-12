/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         GameDataLoader.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Master Game Data Loader
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameDataLoader.generated.h"

UCLASS()
class STARSHATTERWARS_API AGameDataLoader : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGameDataLoader();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
