/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright © 2025-2026. All Rights Reserved.

	ORIGINAL AUTHOR AND STUDIO: John DiCamillo / Destroyer Studios LLC

	SUBSYSTEM:    nGenEx.lib
	FILE:         Sprite.cpp
	AUTHOR:       Carlos Bott


	OVERVIEW
	========
	Sprite (Polygon) Object
*/

#include "Sprite.h"

#include "Math/Vector.h"
#include "Math/UnrealMathUtility.h"
#include "Logging/LogMacros.h"

#include <cstring>

#include "Camera.h"
#include "Polygon.h"
#include "Video.h"
#include "Game.h"

#ifndef STARSHATTERWARS_LOG_DEFINED
#define STARSHATTERWARS_LOG_DEFINED
DECLARE_LOG_CATEGORY_EXTERN(LogStarshatterWars, Log, All);
#endif

// +--------------------------------------------------------------------+

Sprite::Sprite()
	: w(0)
	, h(0)
	, loop(0)
	, nframes(0)
	, own_frames(0)
	, frames(nullptr)
	, frame_index(0)
	, frame_time(100)
	, last_time(0)
	, shade(1.0)
	, angle(0.0)
	, blend_mode(4)
	, filter(1)
	, poly(0)
	, mtl()
	, vset(4)
{
	trans = true;

	vset.space = VertexSet::WORLD_SPACE;
	for (int i = 0; i < 4; i++) {
		vset.diffuse[i] = Color::White.Value();
	}

	vset.tu[0] = 0.0f;  vset.tv[0] = 0.0f;
	vset.tu[1] = 1.0f;  vset.tv[1] = 0.0f;
	vset.tu[2] = 1.0f;  vset.tv[2] = 1.0f;
	vset.tu[3] = 0.0f;  vset.tv[3] = 1.0f;

	poly.nverts = 4;
	poly.vertex_set = &vset;
	poly.material = &mtl;
	poly.verts[0] = 0;
	poly.verts[1] = 1;
	poly.verts[2] = 2;
	poly.verts[3] = 3;
}

// +--------------------------------------------------------------------+

Sprite::Sprite(UTexture2D* animation, int length, int repeat, int share)
	: w(0)
	, h(0)
	, loop(0)
	, nframes(0)
	, own_frames(0)
	, frames(nullptr)
	, frame_index(0)
	, frame_time(67)
	, last_time(0)
	, shade(1.0)
	, angle(0.0)
	, blend_mode(4)
	, filter(1)
	, poly(0)
	, mtl()
	, vset(4)
{
	trans = true;
	SetAnimation(animation, length, repeat, share);

	vset.space = VertexSet::WORLD_SPACE;
	for (int i = 0; i < 4; i++) {
		vset.diffuse[i] = Color::White.Value();
	}

	vset.tu[0] = 0.0f;  vset.tv[0] = 0.0f;
	vset.tu[1] = 1.0f;  vset.tv[1] = 0.0f;
	vset.tu[2] = 1.0f;  vset.tv[2] = 1.0f;
	vset.tu[3] = 0.0f;  vset.tv[3] = 1.0f;

	poly.nverts = 4;
	poly.vertex_set = &vset;
	poly.material = &mtl;
	poly.verts[0] = 0;
	poly.verts[1] = 1;
	poly.verts[2] = 2;
	poly.verts[3] = 3;
}

// +--------------------------------------------------------------------+

Sprite::~Sprite()
{
	// In Unreal, UObjects (UTexture2D) are not deleted manually.
	// Preserve legacy semantics by simply clearing any owned references.
	if (own_frames) {
		frames = nullptr;
		nframes = 0;
		own_frames = 0;
	}
}

// +--------------------------------------------------------------------+

void
Sprite::Scale(double scale)
{
	if (scale >= 0) {
		w = (int)(scale * w);
		h = (int)(scale * h);

		radius = (float)((w > h) ? w : h) / 2.0f;
	}
}

// +--------------------------------------------------------------------+

void
Sprite::Rescale(double scale)
{
	if (scale >= 0 && Frame()) {
		w = (int)(scale * Frame()->Width());
		h = (int)(scale * Frame()->Height());

		radius = (float)((w > h) ? w : h) / 2.0f;
	}
}

// +--------------------------------------------------------------------+

void
Sprite::Reshape(int w1, int h1)
{
	if (w1 >= 0 && h1 >= 0 && Frame()) {
		w = w1;
		h = h1;

		radius = (float)((w > h) ? w : h) / 2.0f;
	}
}

// +--------------------------------------------------------------------+

void
Sprite::SetAnimation(UTexture2D* animation, int length, int repeat, int share)
{
	if (animation) {
		// No filename on UTexture2D; preserve name as-is if caller manages it.
		// Keep legacy safety:
		name[0] = 0;

		// In Unreal, do not delete UObjects. Clear owned refs instead.
		if (own_frames) {
			frames = nullptr;
			nframes = 0;
			own_frames = 0;
		}

		w = animation->Width();
		h = animation->Height();

		radius = (float)((w > h) ? w : h) / 2.0f;

		own_frames = !share;
		nframes = length;
		frames = animation;
		frame_index = 0;

		if (repeat) {
			loop = 1;
			life = -1;
		}
		else {
			loop = 0;
			life = nframes;
		}

		last_time = Game::RealTime() - frame_time;
	}
}

// +--------------------------------------------------------------------+

void
Sprite::SetTexCoords(const double* uv_interleaved)
{
	if (uv_interleaved) {
		vset.tu[0] = (float)uv_interleaved[0];
		vset.tv[0] = (float)uv_interleaved[1];
		vset.tu[1] = (float)uv_interleaved[2];
		vset.tv[1] = (float)uv_interleaved[3];
		vset.tu[2] = (float)uv_interleaved[4];
		vset.tv[2] = (float)uv_interleaved[5];
		vset.tu[3] = (float)uv_interleaved[6];
		vset.tv[3] = (float)uv_interleaved[7];
	}
	else {
		vset.tu[0] = 0.0f;  vset.tv[0] = 0.0f;
		vset.tu[1] = 1.0f;  vset.tv[1] = 0.0f;
		vset.tu[2] = 1.0f;  vset.tv[2] = 1.0f;
		vset.tu[3] = 0.0f;  vset.tv[3] = 1.0f;
	}
}

// +--------------------------------------------------------------------+

double
Sprite::FrameRate() const
{
	return 1000.0 / (double)frame_time;
}

void
Sprite::SetFrameRate(double rate)
{
	if (rate > 0.001 && rate < 100) {
		frame_time = (int)(1000.0 / rate);
	}
}

// +--------------------------------------------------------------------+

void
Sprite::SetFrameIndex(int n)
{
	if (n >= 0 && n < nframes)
		frame_index = n;
}

// +--------------------------------------------------------------------+

UTexture2D*
Sprite::Frame() const
{
	// NOTE: Legacy code treated frames as a contiguous array.
	// In the Unreal port, frames is expected to be resolved by the content system.
	// For now, preserve original behavior for single-frame sprites.
	return frames;
}

// +--------------------------------------------------------------------+

void
Sprite::Render(Video* video, DWORD flags)
{
	if (shade < 0.001 || hidden || !visible || !video)
		return;

	if (blend_mode == 2 && !(flags & Graphic::RENDER_ALPHA))
		return;

	if (blend_mode == 4 && !(flags & Graphic::RENDER_ADDITIVE))
		return;

	if (life > 0 || loop) {
		const Camera* camera = video->GetCamera();
		Matrix        orient(camera->Orientation());
		FVector       nrm(camera->vpn() * -1);

		ColorValue    white((float)shade, (float)shade, (float)shade, (float)shade);
		DWORD         diff = white.ToColor().Value();

		orient.Roll(angle);

		FVector vx =
			FVector((float)orient(0, 0), (float)orient(0, 1), (float)orient(0, 2)) *
			(float)(w / 2.0f);

		FVector vy =
			FVector((float)orient(1, 0), (float)orient(1, 1), (float)orient(1, 2)) *
			(float)(h / 2.0f);

		vset.loc[0] = loc - vx + vy;
		vset.nrm[0] = nrm;
		vset.diffuse[0] = diff;

		vset.loc[1] = loc + vx + vy;
		vset.nrm[1] = nrm;
		vset.diffuse[1] = diff;

		vset.loc[2] = loc + vx - vy;
		vset.nrm[2] = nrm;
		vset.diffuse[2] = diff;

		vset.loc[3] = loc - vx - vy;
		vset.nrm[3] = nrm;
		vset.diffuse[3] = diff;

		if (luminous) {
			mtl.Ka = Color::Black;
			mtl.Kd = Color::Black;
			mtl.Ks = Color::Black;
			mtl.Ke = white;
			mtl.tex_diffuse = Frame();
			mtl.tex_emissive = Frame();
			mtl.blend = blend_mode;
			mtl.luminous = luminous;
		}
		else {
			mtl.Ka = white;
			mtl.Kd = white;
			mtl.Ks = Color::Black;
			mtl.Ke = Color::Black;
			mtl.tex_diffuse = Frame();
			mtl.tex_emissive = 0;
			mtl.blend = blend_mode;
			mtl.luminous = luminous;
		}

		video->DrawPolys(1, &poly);
	}

	std::memset(&screen_rect, 0, sizeof(Rect));
}

// +--------------------------------------------------------------------+

void
Sprite::Render2D(Video* video)
{
	if (shade < 0.001 || hidden || !visible || !video)
		return;

	ColorValue white((float)shade, (float)shade, (float)shade, (float)shade);
	DWORD      diff = white.ToColor().Value();

	double ca = FMath::Cos(Angle());
	double sa = FMath::Sin(Angle());

	double w2 = Width() / 2.0;
	double h2 = Height() / 2.0;

	vset.s_loc[0].x = (float)(loc.X + (-w2 * ca - -h2 * sa) - 0.5);
	vset.s_loc[0].y = (float)(loc.Y + (-w2 * sa + -h2 * ca) - 0.5);
	vset.s_loc[0].z = 0.0f;
	vset.rw[0] = 1.0f;
	vset.diffuse[0] = diff;

	vset.s_loc[1].x = (float)(loc.X + (w2 * ca - -h2 * sa) - 0.5);
	vset.s_loc[1].y = (float)(loc.Y + (w2 * sa + -h2 * ca) - 0.5);
	vset.s_loc[1].z = 0.0f;
	vset.rw[1] = 1.0f;
	vset.diffuse[1] = diff;

	vset.s_loc[2].x = (float)(loc.X + (w2 * ca - h2 * sa) - 0.5);
	vset.s_loc[2].y = (float)(loc.Y + (w2 * sa + h2 * ca) - 0.5);
	vset.s_loc[2].z = 0.0f;
	vset.rw[2] = 1.0f;
	vset.diffuse[2] = diff;

	vset.s_loc[3].x = (float)(loc.X + (-w2 * ca - h2 * sa) - 0.5);
	vset.s_loc[3].y = (float)(loc.Y + (-w2 * sa + h2 * ca) - 0.5);
	vset.s_loc[3].z = 0.0f;
	vset.rw[3] = 1.0f;
	vset.diffuse[3] = diff;

	mtl.Kd = white;
	mtl.tex_diffuse = Frame();
	mtl.blend = blend_mode;

	video->DrawScreenPolys(1, &poly, blend_mode);
}

// +--------------------------------------------------------------------+

void
Sprite::Update()
{
	if (life > 0 || loop) {
		DWORD time = Game::RealTime();
		while (time - last_time > frame_time) {
			life--;
			frame_index++;
			if (frame_index >= nframes)
				frame_index = 0;

			last_time += frame_time;
		}

		if (life < 0 && !loop)
			life = 0;
	}
}
