/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright © 2025-2026. All Rights Reserved.

	ORIGINAL AUTHOR AND STUDIO: John DiCamillo / Destroyer Studios LLC

	SUBSYSTEM:    Stars.exe
	FILE:         StarSystem.h
	AUTHOR:       Carlos Bott


	OVERVIEW
	========
	Various heavenly bodies
*/

#pragma once

#include "Types.h"
#include "Text.h"
#include "Term.h"
#include "List.h"

#include "Math/Vector.h"
#include "Math/Color.h"

// +--------------------------------------------------------------------+

class UTexture2D;

class StarSystem;
class Orbital;
class OrbitalBody;
class OrbitalRegion;
class TerrainRegion;

class Graphic;
class SimLight;
class SimScene;
class Solid;

// +--------------------------------------------------------------------+

class StarSystem
{
public:
	static const char* TYPENAME() { return "StarSystem"; }

	StarSystem(const char* name, FVector loc, int iff = 0, int s = 4);
	virtual ~StarSystem();

	int operator == (const StarSystem& s) const { return name == s.name; }

	// operations:
	virtual void      Load();
	virtual void      Create();
	virtual void      Destroy();

	virtual void      Activate(SimScene& scene);
	virtual void      Deactivate();

	virtual void      ExecFrame();

	// accessors:
	const char* Name()         const { return name; }
	const char* Govt()         const { return govt; }
	const char* Description()  const { return description; }
	int               Affiliation()  const { return affiliation; }
	int               Sequence()     const { return seq; }
	FVector           Location()     const { return loc; }
	int               NumStars()     const { return sky_stars; }
	int               NumDust()      const { return sky_dust; }
	FColor            Ambient()      const;

	List<OrbitalBody>& Bodies() { return bodies; }
	List<OrbitalRegion>& Regions() { return regions; }
	List<OrbitalRegion>& AllRegions() { return all_regions; }
	OrbitalRegion* ActiveRegion() { return active_region; }

	Orbital* FindOrbital(const char* name);
	OrbitalRegion* FindRegion(const char* name);

	void              SetActiveRegion(OrbitalRegion* rgn);

	static void       SetBaseTime(double t, bool absolute = false);
	static double     GetBaseTime();
	static double     Stardate() { return stardate; }
	static void       CalcStardate();
	double            Radius()       const { return radius; }

	void              SetSunlight(FColor color, double brightness = 1);
	void              SetBacklight(FColor color, double brightness = 1);
	void              RestoreTrueSunColor();
	bool              HasLinkTo(StarSystem* s) const;
	const Text& GetDataPath() const { return datapath; }

protected:
	void              ParseStar(TermStruct* val);
	void              ParsePlanet(TermStruct* val);
	void              ParseMoon(TermStruct* val);
	void              ParseRegion(TermStruct* val);
	void              ParseTerrain(TermStruct* val);
	void              ParseLayer(TerrainRegion* rgn, TermStruct* val);

	void              CreateBody(OrbitalBody& body);
	FVector           TerrainTransform(const FVector& loc);

	char              filename[64];
	Text              name;
	Text              govt;
	Text              description;
	Text              datapath;
	int               affiliation;
	int               seq;
	FVector           loc;
	static double     stardate;
	double            radius;
	bool              instantiated;

	int               sky_stars;
	int               sky_dust;
	Text              sky_poly_stars;
	Text              sky_nebula;
	Text              sky_haze;
	double            sky_uscale;
	double            sky_vscale;
	FColor            ambient;
	FColor            sun_color;
	double            sun_brightness;
	double            sun_scale;
	List<SimLight>       sun_lights;
	List<SimLight>       back_lights;

	Graphic* point_stars;
	Solid* poly_stars;
	Solid* nebula;
	Solid* haze;

	List<OrbitalBody>    bodies;
	List<OrbitalRegion>  regions;
	List<OrbitalRegion>  all_regions;

	Orbital* center;
	OrbitalRegion* active_region;

	FVector           tvpn, tvup, tvrt;
};

// +--------------------------------------------------------------------+

class Star
{
public:
	static const char* TYPENAME() { return "Star"; }

	Star(const char* n, const FVector& l, int s) : name(n), loc(l), seq(s) {}
	virtual ~Star() {}

	enum SPECTRAL_CLASS {
		BLACK_HOLE, WHITE_DWARF, RED_GIANT,
		O, B, A, F, G, K, M
	};

	int operator == (const Star& s)     const { return name == s.name; }

	// accessors:
	const char* Name()         const { return name; }
	const FVector& Location()     const { return loc; }
	int               Sequence()     const { return seq; }
	FColor            GetColor()     const;
	int               GetSize()      const;

	static FColor     GetColor(int spectral_class);
	static int        GetSize(int spectral_class);

protected:
	Text              name;
	FVector           loc;
	int               seq;
};

// +--------------------------------------------------------------------+

class Orbital 
{
	friend class StarSystem;

public:
	static const char* TYPENAME() { return "Orbital"; }

	enum OrbitalType { NOTHING, STAR, PLANET, MOON, REGION, TERRAIN };

	Orbital(StarSystem* sys, const char* n, OrbitalType t, double m, double r, double o, Orbital* p = 0);
	virtual ~Orbital();

	int operator == (const Orbital& o) const { return type == o.type && name == o.name && system == o.system; }
	int operator <  (const Orbital& o) const { return loc.Size() < o.loc.Size(); }
	int operator <= (const Orbital& o) const { return loc.Size() <= o.loc.Size(); }

	// operations:
	virtual void      Update();
	FVector           PredictLocation(double delta_t);

	// accessors:
	const char* Name()         const { return name; }
	OrbitalType       Type()         const { return type; }
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
	FVector           Location()     const { return loc; }
	Graphic* Rep()          const { return rep; }

	UTexture2D* GetMapIcon()   const { return map_icon; }
	void              SetMapIcon(UTexture2D* img) { map_icon = img; }

	StarSystem* System()       const { return system; }
	Orbital* Primary()      const { return primary; }
	ListIter<OrbitalRegion> Regions() { return regions; }

protected:
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
	FVector           loc;
	bool              retro;
	Graphic* rep;
	UTexture2D* map_icon;

	StarSystem* system;
	Orbital* primary;

	List<OrbitalRegion>   regions;
};

// +--------------------------------------------------------------------+

class OrbitalBody : public Orbital
{
	friend class StarSystem;

public:
	static const char* TYPENAME() { return "OrbitalBody"; }

	OrbitalBody(StarSystem* sys, const char* n, OrbitalType t, double m, double r, double o, Orbital* prime = 0);
	virtual ~OrbitalBody();

	// operations:
	virtual void      Update();

	// accessors:
	ListIter<OrbitalBody>   Satellites() { return satellites; }

	double                  Tilt()     const { return tilt; }
	double                  RingMin()  const { return ring_min; }
	double                  RingMax()  const { return ring_max; }

	double                  LightIntensity() const { return light; }
	FColor                  LightColor()     const { return color; }
	bool                    Luminous()       const { return luminous; }

protected:
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
	SimLight*		  light_rep;
	SimLight*	      back_light;
	FColor            color;
	FColor            back;
	FColor            atmosphere;
	bool              luminous;

	List<OrbitalBody> satellites;
};

// +--------------------------------------------------------------------+

class OrbitalRegion : public Orbital
{
	friend class StarSystem;

public:
	static const char* TYPENAME() { return "OrbitalRegion"; }

	OrbitalRegion(StarSystem* sys, const char* n, double m, double r, double o, Orbital* prime = 0);
	virtual ~OrbitalRegion();

	double            GridSpace()       const { return grid; }
	double            Inclination()     const { return inclination; }
	int               Asteroids()       const { return asteroids; }
	List<Text>& Links() { return links; }

protected:
	double            grid;
	double            inclination;
	int               asteroids;
	List<Text>        links;
};
