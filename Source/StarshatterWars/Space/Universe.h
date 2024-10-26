/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Space
	FILE:         Universe.h
	AUTHOR:       Carlos Bott
*/


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Foundation/Text.h"
#include "../Foundation/Term.h"
#include "../Foundation/List.h"

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/DataTable.h"

#include "Universe.generated.h"

UCLASS()
class STARSHATTERWARS_API AUniverse : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AUniverse();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FString ProjectPath;
	FString FilePath;
	
	void SpawnGalaxy();
};
