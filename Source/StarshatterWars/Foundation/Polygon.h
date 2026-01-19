/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright (C) 2025-2026. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         Polygon.h
	AUTHOR:       Carlos Bott
	ORIGINAL:     John DiCamillo / Destroyer Studios LLC

	OVERVIEW
	========
	Polygon structures: VertexSet, Poly, Material
*/

#ifndef Polygon_h
#define Polygon_h

#include "Geometry.h"
#include "Color.h"

// Minimal Unreal include (required by request: convert Vec3/Point to FVector):
#include "Math/Vector.h"

// +--------------------------------------------------------------------+
// Forward declarations (keep header light)

class UTexture2D;

struct Poly;
struct Material;
struct VertexSet;

// +--------------------------------------------------------------------+

struct Poly
{
	static const char* TYPENAME() { return "Poly"; }

	enum { MAX_VERTS = 4 };

	Poly() {}
	Poly(int init);
	~Poly() {}

	int operator <  (const Poly& p) const { return sortval < p.sortval; }
	int operator == (const Poly& p) const { return this == &p; }

	int Contains(const FVector& pt) const;

	BYTE           nverts;
	BYTE           visible;
	WORD           verts[MAX_VERTS];
	WORD           vlocs[MAX_VERTS];
	VertexSet* vertex_set;
	Material* material;
	int            sortval;
	float          flatness;
	Plane          plane;
};

// +--------------------------------------------------------------------+

struct Material
{
	static const char* TYPENAME() { return "Material"; }

	enum BLEND_TYPE { MTL_SOLID = 1, MTL_TRANSLUCENT = 2, MTL_ADDITIVE = 4 };
	enum { NAMELEN = 32 };

	Material();
	~Material();

	int operator == (const Material& m) const;

	void Clear();

	char        name[NAMELEN];
	char        shader[NAMELEN];

	ColorValue  Ka;         // ambient color
	ColorValue  Kd;         // diffuse color
	ColorValue  Ks;         // specular color
	ColorValue  Ke;         // emissive color
	float       power;      // highlight sharpness (big=shiny)
	float       brilliance; // diffuse power function
	float       bump;       // bump level (0=none)
	DWORD       blend;      // alpha blend type
	bool        shadow;     // casts shadow
	bool        luminous;   // verts have their own lighting

	// Render assets: Bitmap -> UTexture2D*
	UTexture2D* tex_diffuse;
	UTexture2D* tex_specular;
	UTexture2D* tex_bumpmap;
	UTexture2D* tex_emissive;
	UTexture2D* tex_alternate;
	UTexture2D* tex_detail;

	bool        IsSolid()         const { return blend == MTL_SOLID; }
	bool        IsTranslucent()   const { return blend == MTL_TRANSLUCENT; }
	bool        IsGlowing()       const { return blend == MTL_ADDITIVE; }
	const char* GetShader(int n)  const;

	//
	// Support for Magic GUI
	//

	Color       ambient_color;
	Color       diffuse_color;
	Color       specular_color;
	Color       emissive_color;

	float       ambient_value;
	float       diffuse_value;
	float       specular_value;
	float       emissive_value;

	// Render assets: Bitmap -> UTexture2D*
	UTexture2D* thumbnail;        // preview image

	void        CreateThumbnail(int size = 128);
	DWORD       GetThumbColor(int i, int j, int size);
};

// +--------------------------------------------------------------------+

struct VertexSet
{
	static const char* TYPENAME() { return "VertexSet"; }

	enum VertexSpaces { OBJECT_SPACE, WORLD_SPACE, VIEW_SPACE, SCREEN_SPACE };

	VertexSet(int m);
	~VertexSet();

	void     Resize(int m, bool preserve = false);
	void     Delete();
	void     Clear();
	void     CreateTangents();
	void     CreateAdditionalTexCoords();
	bool     CopyVertex(int dst, int src);
	void     CalcExtents(FVector& plus, FVector& minus);

	VertexSet* Clone() const;

	int      nverts;
	int      space;

	// Geometry: Vec3 -> FVector
	FVector* loc;
	FVector* nrm;
	FVector* s_loc;

	float* rw;
	float* tu;
	float* tv;
	float* tu1;
	float* tv1;
	DWORD* diffuse;
	DWORD* specular;

	// Tangent basis: Vec3 -> FVector
	FVector* tangent;
	FVector* binormal;
};

#endif // Polygon_h
