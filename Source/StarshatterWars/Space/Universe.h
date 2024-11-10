/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Space
	FILE:         Universe.h
	AUTHOR:       Carlos Bott
*/


#pragma once

#include "CoreMinimal.h"
#include "../Foundation/Text.h"
#include "../Foundation/Term.h"
#include "../Foundation/List.h"

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/DataTable.h"
#include "UObject/NoExportTypes.h"
#include "Tickable.h"
#include "../System/SSWGameInstance.h"
#include "Universe.generated.h"

UCLASS()
class STARSHATTERWARS_API UUniverse : public UObject, public FTickableGameObject
{
	GENERATED_BODY()
	
public:	
	static const char* TYPENAME() { return "Universe"; }
	
	// Sets default values for this actor's properties
	UUniverse();

	// Called every frame
	void Tick(float DeltaTime) override;
	bool IsTickable() const override;
	bool IsTickableInEditor() const override;
	bool IsTickableWhenPaused() const override;
	TStatId GetStatId() const override;

	UWorld* GetWorld() const override;
	FString ProjectPath;
	FString FilePath;
	
	void SpawnGalaxy();
};
