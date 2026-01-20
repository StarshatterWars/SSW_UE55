/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright © 2025-2026. All Rights Reserved.

	ORIGINAL AUTHOR AND STUDIO: John DiCamillo / Destroyer Studios LLC

	SUBSYSTEM:    Stars.exe
	FILE:         Hoop.h
	AUTHOR:       Carlos Bott


	OVERVIEW
	========
	ILS Hoop (HUD display) class
*/

#pragma once

#include "Types.h"
#include "Solid.h"

// Forward declarations to keep the header light:
class UTexture2D;
class Material;

class Hoop : public Solid
{
public:
	Hoop();
	virtual ~Hoop();

	virtual void      Update();
	static  void      SetColor(Color c);

protected:
	virtual void      CreatePolys();

	UTexture2D*		  hoop_texture;
	Material*		  mtl;
	int               width;
	int               height;
};

