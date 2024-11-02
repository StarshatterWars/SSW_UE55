// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
//#include "Solid.h"
//#include "Bitmap.h"
#include "Star.h"
#include "../Foundation/Geometry.h"
#include "../Foundation/Color.h"
#include "../Foundation/Text.h"
#include "../Foundation/Term.h"
#include "../Foundation/List.h"

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/DataTable.h"

#include "StarSystem.generated.h"

class AStarSystem;
class Color;
class DataLoader;
class AOrbital;
class AOrbitalBody;
class AOrbitalRegion;
class TerrainRegion;

//class Graphic;
//class Light;
//class Scene;

UCLASS()
class STARSHATTERWARS_API AStarSystem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AStarSystem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//virtual void      Activate(Scene& scene);
	//virtual void      Deactivate();

	virtual void      ExecFrame();
	
	// operations:
	virtual void      Load();
	void			  Load(const char* FileName);
	virtual void      Initialize(const char* Name);
	virtual void      Create();
	virtual void      Destroy();

	// accessors:
	const char* Name()         const { return name; }
	const char* Govt()         const { return govt; }
	const char* Description()  const { return description; }
	int Affiliation()  const { return affiliation; }
	int Sequence()     const { return seq; }
	Point Location()     const { return loc; }
	int NumStars()     const { return sky_stars; }
	int NumDust()      const { return sky_dust; }
	Color Ambient()      const;

	List<AOrbitalBody>&   Bodies()       { return bodies;  }
	List<AOrbitalRegion>& Regions()      { return regions; }
	List<AOrbitalRegion>& AllRegions()   { return all_regions;   }
	AOrbitalRegion*       ActiveRegion() { return active_region; }

	AOrbital*          FindOrbital(const char* name);
	AOrbitalRegion*    FindRegion(const char* name);

	void              SetActiveRegion(AOrbitalRegion* rgn);

	UFUNCTION()
	static void SetBaseTime(double t, bool absolute = false);
	UFUNCTION()
	static double GetBaseTime();
	static double Stardate() { return StarDate; }

	UFUNCTION()
	static void CalcStardate();
	UFUNCTION()
	double Radius()       const { return radius; }

	void SetSunlight(Color color, double brightness = 1);
	void SetBacklight(Color color, double brightness = 1);
	void RestoreTrueSunColor();
	bool HasLinkTo(AStarSystem* s) const;
	FString GetDataPath() const { return DataPath; }

	static double StarDate;

protected:
	void              ParseStar(TermStruct* val);
	void              ParsePlanet(TermStruct* val);
	void              ParseMoon(TermStruct* val);
	void              ParseRegion(TermStruct* val);
	void              ParseTerrain(TermStruct* val);
	void			  SpawnStar(FString Name, double m, double rad, double o, double r);
	void			  SpawnPlanet(FString Name, double m, double rad, double o, double r);
	void			  SpawnMoon(FString Name, double m, double rad, double o, double r);
	//void              ParseLayer(TerrainRegion* rgn, TermStruct* val);

	//void              CreateBody(OrbitalBody& body);
	//Point             TerrainTransform(const Point& loc);

	// +--------------------------------------------------------------------+

	char              filename[64];
	Text              name;
	Text              govt;
	Text              description;
	FString           DataPath;
	int               affiliation;
	int               seq;
	Point             loc;
	
	double            radius;
	bool              instantiated;
	
	int               sky_stars;
	int               sky_dust;
	Text              sky_poly_stars;
	Text              sky_nebula;
	Text              sky_haze;
	double            sky_uscale;
	double            sky_vscale;
	Color             ambient;
	Color             sun_color;
	double            sun_brightness;
	double            sun_scale;
	
	//List<Light>       sun_lights;
	//List<Light>       back_lights;

	//Graphic*          point_stars;
	//Solid*            poly_stars;
	//Solid*            nebula;
	//Solid*            haze;

	List<AOrbitalBody>    bodies;
	List<AOrbitalRegion>  regions;
	List<AOrbitalRegion>  all_regions;

	AOrbital*          center;
	AOrbitalRegion*    active_region;

	Point             tvpn, tvup, tvrt;
	
	FString ProjectPath;
	FString FilePath;

	UPROPERTY()
	USceneComponent* Root;

	AOrbitalBody* Parent;
	AOrbitalBody* StarParent;
	AOrbitalBody* PlanetParent;
	AOrbitalBody* MoonParent;
	AStarSystem* SystemParent;
};

// +--------------------------------------------------------------------+


