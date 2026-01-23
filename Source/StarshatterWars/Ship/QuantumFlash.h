/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright © 2025–2026. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         QuantumFlash.h
	AUTHOR:       Carlos Bott
	ORIGINAL:     John DiCamillo / Destroyer Studios LLC

	OVERVIEW
	========
	Quantum Warp Out special effect class
*/

#pragma once

// --------------------------------------------------------------------
// Forward declarations (keep header light)
// --------------------------------------------------------------------

class Video;
struct Material;
struct VertexSet;
struct Poly;
class Bitmap;
class SimObject;
struct Matrix;
class SystemFont;

// --------------------------------------------------------------------
// Minimal Unreal includes
// --------------------------------------------------------------------

#include "Math/Vector.h"                // FVector
#include "Math/Color.h"                 // FColor
#include "Math/UnrealMathUtility.h"     // Math utilities

// --------------------------------------------------------------------
// Starshatter core includes
// --------------------------------------------------------------------

#include "Types.h"
#include "Graphic.h"

// +--------------------------------------------------------------------+

class QuantumFlash : public Graphic
{
public:
	QuantumFlash();
	virtual ~QuantumFlash();

	// operations
	virtual void   Render(Video* video, DWORD flags) override;

	// accessors / mutators
	virtual void   SetOrientation(const Matrix& o);
	void           SetDirection(const FVector& v);
	void           SetEndPoints(const FVector& from, const FVector& to);

	double         Shade() const { return shade; }
	void           SetShade(double s);

protected:
	void           UpdateVerts(const FVector& cam_pos);

	double         length;
	double         width;
	double         shade;

	int            npolys;
	int            nverts;

	Material* mtl;
	VertexSet* verts;
	Poly* polys;
	Matrix* beams;
	Bitmap* texture;

	Matrix         orientation;
};
