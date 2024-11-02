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

class StarSystem;
class Color;
class DataLoader;
class Orbital;
class OrbitalBody;
class OrbitalRegion;
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

	List<OrbitalBody>&   Bodies()       { return bodies;  }
	List<OrbitalRegion>& Regions()      { return regions; }
	List<OrbitalRegion>& AllRegions()   { return all_regions;   }
	OrbitalRegion*       ActiveRegion() { return active_region; }

	Orbital*          FindOrbital(const char* name);
	OrbitalRegion*    FindRegion(const char* name);

	void              SetActiveRegion(OrbitalRegion* rgn);

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
	bool HasLinkTo(StarSystem* s) const;
	FString GetDataPath() const { return DataPath; }

	static double StarDate;

protected:
	void              ParseStar(TermStruct* val);
	void              ParsePlanet(TermStruct* val);
	void              ParseMoon(TermStruct* val);
	void              ParseRegion(TermStruct* val);
	void              ParseTerrain(TermStruct* val);
	void			  SpawnStar(FString starName, EOrbitalType t, double m, double r, double o);
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

	List<OrbitalBody>    bodies;
	List<OrbitalRegion>  regions;
	List<OrbitalRegion>  all_regions;

	Orbital*          center;
	OrbitalRegion*    active_region;

	Point             tvpn, tvup, tvrt;
	
	FString ProjectPath;
	FString FilePath;

	UPROPERTY()
	USceneComponent* Root;
};

// +--------------------------------------------------------------------+

class Star 
{

public:
	static const char* TYPENAME() { return "Star"; }

	Star(const char* n, const Point& l, int s) : name(n), loc(l), seq(s) { }
	virtual ~Star() { }

	int operator == (const Star& s)     const { return name == s.name; }

	// accessors:
	const char*       Name()         const { return name;       }
	const Point&      Location()     const { return loc;        }
	int               Sequence()     const { return seq;        }
	Color             GetColor()     const;
	int               GetSize()      const;

	static Color      GetColor(int spectral_class);
	static int        GetSize(int spectral_class);

protected:
	Text              name;
	Point             loc;
	int               seq;
};

// +--------------------------------------------------------------------+

class Orbital
{
	friend class StarSystem;

public:
	static const char* TYPENAME() { return "Orbital"; }

	enum OrbitalType     { NOTHING, STAR, PLANET, MOON, REGION, TERRAIN };

	Orbital(StarSystem* sys, const char* n, OrbitalType t, double m, double r, double o, Orbital* p=0);
	virtual ~Orbital();

	int operator == (const Orbital& o) const { return type  == o.type && name == o.name && system == o.system; }
	int operator <  (const Orbital& o) const { return loc.length() <  o.loc.length(); }
	int operator <= (const Orbital& o) const { return loc.length() <= o.loc.length(); }

	// operations:
	virtual void      Update();
	Point             PredictLocation(double delta_t);

	// accessors:
	const char*       Name()         const { return name;       }
	OrbitalType       Type()         const { return type;       }
	int               SubType()      const { return subtype;    }

	const char*       Description()  const { return description; }
	double            Mass()         const { return mass;       }
	double            Radius()       const { return radius;     }
	double            Rotation()     const { return rotation;   }
	double            RotationPhase()const { return theta;      }
	double            Orbit()        const { return orbit;      }
	bool              Retrograde()   const { return retro;      }
	double            Phase()        const { return phase;      }
	double            Period()       const { return period;     }
	Point             Location()     const { return loc;        }
	//Graphic*          Rep()          const { return rep;        }

	//const Bitmap&     GetMapIcon()   const { return map_icon;   }
	//void              SetMapIcon(const Bitmap& img);

	StarSystem*       System()       const { return system;     }
	Orbital*          Primary()      const { return primary;    }
	ListIter<OrbitalRegion> Regions()      { return regions;    }

	Text              name;
	OrbitalType       type;
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

	StarSystem*       system;
	Orbital*          primary;

	List<OrbitalRegion>   regions;
};

// +--------------------------------------------------------------------+

class OrbitalBody : public Orbital
{
	friend class StarSystem;

public:
	static const char* TYPENAME() { return "OrbitalBody"; }

	OrbitalBody(AStarSystem* sys, const char* n, OrbitalType t, double m, double r, double o, Orbital* prime=0);
	virtual ~OrbitalBody();

	// operations:
	virtual void      Update();

	// accessors:
	ListIter<OrbitalBody>   Satellites()     { return satellites; }

	double                  Tilt()     const { return tilt; }
	double                  RingMin()  const { return ring_min; }
	double                  RingMax()  const { return ring_max; }

	double                  LightIntensity() const { return light;    }
	Color                   LightColor()     const { return color;    }
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

	List<OrbitalBody> satellites;
};

// +--------------------------------------------------------------------+

class OrbitalRegion : public Orbital
{
	friend class StarSystem;

public:
	static const char* TYPENAME() { return "OrbitalRegion"; }

	OrbitalRegion(StarSystem* sys, const char* n, double m, double r, double o, Orbital* prime=0);
	virtual ~OrbitalRegion();

	double            GridSpace()       const { return grid;          }
	double            Inclination()     const { return inclination;   }
	int               Asteroids()       const { return asteroids;     }
	List<Text>&       Links()                 { return links;         }

protected:
	double            grid;
	double            inclination;
	int               asteroids;
	List<Text>        links;
};

	

