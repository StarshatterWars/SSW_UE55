// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Foundation/Geometry.h"
#include "../Foundation/Color.h"
#include "../Foundation/Text.h"
#include "../Foundation/Term.h"
#include "../Foundation/List.h"
#include "Starsystem.h"
#include "../System/SSWGameInstance.h"
#include "Orbital.generated.h"

class AOrbitalRegion;

UCLASS()
class STARSHATTERWARS_API AOrbital : public AActor
{
	GENERATED_BODY()
	
public:	
	static const char* TYPENAME() { return "Orbital"; }
	
	// Sets default values for this actor's properties
	AOrbital();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY()
	USceneComponent* Root;

	int operator == (const AOrbital& o) const { return type == o.type && name == o.name && system == o.system; }
	int operator <  (const AOrbital& o) const { return loc.length() < o.loc.length(); }
	int operator <= (const AOrbital& o) const { return loc.length() <= o.loc.length(); }

	// operations:
	virtual void      Update();
	Point             PredictLocation(double delta_t);

	// accessors:
	const char* Name()         const { return name; }
	EOrbitalType      Type()         const { return type; }
	int               SubType()      const { return subtype; }

	const char* Description()  const { return description; }
	double            Mass()         const { return mass; }
	double            Radius()       const { return radius; }
	double            Rotation()     const { return rotation; }
	double            RotationPhase()const { return theta; }
	double            Orbit()        const { return orbit; }
	bool              Retrograde()   const { return retro; }
	double            Phase()        const { return phase; }
	double            Period()       const { return period; }
	Point             Location()     const { return loc; }
	//Graphic*          Rep()          const { return rep;        }

	//const Bitmap&     GetMapIcon()   const { return map_icon;   }
	//void              SetMapIcon(const Bitmap& img);

	StarSystem* System()       const { return system; }
	Orbital* Primary()      const { return primary; }
	ListIter<AOrbitalRegion> Regions() { return regions; }

	Text              name;
	EOrbitalType      type;
	int               subtype;

	Text              description;
	double            mass;
	double            radius;
	double            rotation;
	double            theta;
	double            orbit;
	double            phase;
	double            period;
	double            velocity;
	Point             loc;
	bool              retro;
	//Graphic*          rep;
	//Bitmap            map_icon;

	StarSystem* system;
	Orbital* primary;

	List<AOrbitalRegion>   regions;
};


