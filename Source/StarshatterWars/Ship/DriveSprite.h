/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright © 2025-2026. All Rights Reserved.

	ORIGINAL AUTHOR AND STUDIO: John DiCamillo / Destroyer Studios LLC

	SUBSYSTEM:    Stars.exe
	FILE:         DriveSprite.h
	AUTHOR:       Carlos Bott


	OVERVIEW
	========
	Specialized Drive Sprite Object
*/

#pragma once

#include "Types.h"
#include "Sprite.h"

#include "Math/Vector.h"

// Forward declarations to keep the header light:
class UTexture2D;
class Video;

class DriveSprite : public Sprite
{
public:
	DriveSprite();
	DriveSprite(UTexture2D* animation, UTexture2D* glow);
	DriveSprite(UTexture2D* animation, int length = 1, int repeat = 1, int share = 1);
	virtual ~DriveSprite();

	// operations
	virtual void   Render(Video* video, DWORD flags);
	virtual void   SetFront(const FVector& f);
	virtual void   SetBias(DWORD b);

protected:
	double			effective_radius;
	FVector			front;
	UTexture2D*		glow;
	DWORD			bias;
};
