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
	static const char* TYPENAME() { return "OrbitalBody"; }

	// Sets default values for this actor's properties
	AOrbitalBody();

	void Initialize(AStarSystem* sys, FString n, EOrbitalType t, double m, double r, double o, AOrbital* prime = nullptr);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	

	// operations:
	virtual void      Update();

	// accessors:
	ListIter<AOrbitalBody>   Satellites() { return satellites; }

	double                  Tilt()     const { return tilt; }
	double                  RingMin()  const { return ring_min; }
	double                  RingMax()  const { return ring_max; }

	double                  LightIntensity() const { return light; }
	Color                   LightColor()     const { return color; }
	bool                    Luminous()       const { return luminous; }

	Text              map_name;
	Text              tex_name;
	Text              tex_high_res;
	Text              tex_ring;
	Text              tex_glow;
	Text              tex_glow_high_res;
	Text              tex_gloss;

	double            tscale;
	double            light;
	double            ring_min;
	double            ring_max;
	double            tilt;
	//Light*            light_rep;
	//Light*            back_light;
	Color             color;
	Color             back;
	Color             atmosphere;
	bool              luminous;

	List<AOrbitalBody> satellites;
};
