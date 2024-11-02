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
	int               SubType()      const { return (int8) subtype; }

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

	AStarSystem* System()       const { return system; }
	AOrbital* Primary()      const { return primary; }
	ListIter<AOrbitalRegion> Regions() { return regions; }

	Text              name;
	EOrbitalType      type;
	ESPECTRAL_CLASS   subtype;

	Text              description;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	double            mass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	double            radius;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	double            rotation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	double            theta;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	double            orbit;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	double            phase;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	double            period;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	double            velocity;
	
	Point             loc;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool              retro;
	//Graphic*          rep;
	//Bitmap            map_icon;

	AStarSystem* system;
	AOrbital* primary;

	List<AOrbitalRegion>   regions;
};


