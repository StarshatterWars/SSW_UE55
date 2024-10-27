/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Space
	FILE:         Star.h
	AUTHOR:       Carlos Bott
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Foundation/Geometry.h"
#include "../Foundation/Color.h"
#include "../Foundation/Text.h"
#include "../Foundation/Term.h"
#include "../Foundation/List.h"
#include "../System/SSWGameInstance.h"

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/DataTable.h"

#include "Star.generated.h"

UCLASS()
class STARSHATTERWARS_API AStar : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AStar();

	static const char* TYPENAME() { return "Star"; }

	AStar(const char* n, const Point& l, int s) : name(n), loc(l), seq(s) { }
	virtual ~AStar() { }

	int operator == (const AStar& s)     const { return name == s.name; }
	void Initialize(const char* n, const Point& l, int s);

	// accessors:
	const char* Name()         const { return name; }
	const Point& Location()     const { return loc; }
	int               Sequence()     const { return seq; }
	Color             GetColor()     const;
	int               GetSize()      const;

	static Color      GetColor(ESPECTRAL_CLASS spectral_class);
	static int        GetSize(ESPECTRAL_CLASS spectral_class);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	Text              name;
	Point             loc;
	int               seq;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
};
