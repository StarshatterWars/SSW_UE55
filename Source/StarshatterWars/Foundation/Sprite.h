/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright © 2025-2026. All Rights Reserved.

	ORIGINAL AUTHOR AND STUDIO: John DiCamillo / Destroyer Studios LLC

	SUBSYSTEM:    nGenEx.lib
	FILE:         Sprite.h
	AUTHOR:       Carlos Bott


	OVERVIEW
	========
	Sprite Object
*/

#pragma once

#include "Types.h"
#include "Graphic.h"
#include "Polygon.h"

class UTexture2D;

class Sprite : public Graphic
{
public:
	static const char* TYPENAME() { return "Sprite"; }

	Sprite();
	Sprite(UTexture2D* animation, int length = 1, int repeat = 1, int share = 1);
	virtual ~Sprite();

	// operations
	virtual void   Render(Video* video, DWORD flags);
	virtual void   Render2D(Video* video);
	virtual void   Update();
	virtual void   Scale(double scale);
	virtual void   Rescale(double scale);
	virtual void   Reshape(int w1, int h1);

	// accessors / mutators
	int            Width()     const { return w; }
	int            Height()    const { return h; }
	int            Looping()   const { return loop; }
	int            NumFrames() const { return nframes; }
	double         FrameRate() const;
	void           SetFrameRate(double rate);

	double         Shade()     const { return shade; }
	void           SetShade(double s) { shade = s; }
	double         Angle()     const { return angle; }
	void           SetAngle(double a) { angle = a; }
	int            BlendMode() const { return blend_mode; }
	void           SetBlendMode(int a) { blend_mode = a; }
	int            Filter()    const { return filter; }
	void           SetFilter(int f) { filter = f; }
	virtual void   SetAnimation(UTexture2D* animation, int length = 1, int repeat = 1, int share = 1);
	virtual void   SetTexCoords(const double* uv_interleaved);

	UTexture2D* Frame()     const;
	void           SetFrameIndex(int n);

	virtual bool   IsSprite()  const { return true; }

protected:
	int            w, h;
	int            loop;

	int            nframes;
	int            own_frames;
	UTexture2D* frames;
	int            frame_index;
	DWORD          frame_time;
	DWORD          last_time;
	double         shade;
	double         angle;
	int            blend_mode;
	int            filter;

	Poly           poly;
	Material       mtl;
	VertexSet      vset;
};
