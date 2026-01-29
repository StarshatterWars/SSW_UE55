/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright (C) 2025-2026. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         SimLight.h
	AUTHOR:       Carlos Bott
	ORIGINAL:     John DiCamillo / Destroyer Studios LLC

	OVERVIEW
	========
	Dynamic Light Source
*/
#pragma once
#include "Geometry.h"

// Minimal Unreal include (required by request: convert Point/Vec3 to FVector):
#include "Math/Vector.h"
#include "Math/Color.h"
#include "GameStructs.h"

// +--------------------------------------------------------------------+

#define SIMLIGHT_DESTROY(x) if (x) { x->Destroy(); x = 0; }

// +--------------------------------------------------------------------+
// Forward declarations (keep header light)

class UTexture2D;
class SimScene;

// +--------------------------------------------------------------------+

class SimLight
{
public:
	static const char* TYPENAME() { return "SimLight"; }

	SimLight(float l = 0.0f, float dl = 1.0f, int time = -1);
	virtual ~SimLight();

	int operator == (const SimLight& l) const { return id == l.id; }

	// operations
	virtual void      Update();

	// accessors / mutators
	int               Identity()        const { return id; }
	FVector           Location()        const { return loc; }

	LIGHTTYPE Type() const { return static_cast<LIGHTTYPE>(type); }
	void              SetType(LIGHTTYPE t) { type = t; }
	float             Intensity()       const { return light; }
	void              SetIntensity(float f) { light = f; }
	FColor            GetColor()        const { return color; }
	void              SetColor(FColor c) { color = c; }
	bool              IsActive()        const { return active; }
	void              SetActive(bool a) { active = a; }
	bool              CastsShadow()     const { return shadow; }
	void              SetShadow(bool s) { shadow = s; }

	bool              IsPoint()         const { return type == LIGHTTYPE::POINT; }
	bool              IsSpot()          const { return type == LIGHTTYPE::SPOT; }
	bool              IsDirectional()   const { return type == LIGHTTYPE::DIRECTIONAL; }

	virtual void      MoveTo(const FVector& dst);
	virtual void      TranslateBy(const FVector& ref);

	virtual int       Life()            const { return life; }
	virtual void      Destroy();
	virtual SimScene* GetScene()        const { return scene; }
	virtual void      SetScene(SimScene* s) { scene = s; }

protected:
	static int        id_key;

	int               id;
	LIGHTTYPE         type;
	FVector           loc;
	int               life;
	float             light;
	float             dldt;
	FColor            color;
	bool              active;
	bool              shadow;
	SimScene* scene;
};

