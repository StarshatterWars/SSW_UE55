/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright (c) 2025-2026. All Rights Reserved.

	ORIGINAL AUTHOR AND STUDIO
	==========================
	John DiCamillo / Destroyer Studios LLC

	SUBSYSTEM:    nGenEx.lib
	FILE:         Sprite.cpp
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Sprite (Polygon) Object
*/

#include "Sprite.h"

#include "Bitmap.h"
#include "Camera.h"
#include "Video.h"
#include "Game.h"

// Minimal Unreal includes:
#include "Logging/LogMacros.h"
#include "Math/Vector.h"
#include "Math/Color.h"
#include "Math/Matrix.h"
#include "Math/Quat.h"

#include <cmath>
#include <cstring>

DEFINE_LOG_CATEGORY_STATIC(LogStarshatterWars, Log, All);

// +--------------------------------------------------------------------+

static uint8 ClampColorByte(double v01)
{
	if (v01 <= 0.0) return 0;
	if (v01 >= 1.0) return 255;
	return (uint8)(v01 * 255.0 + 0.5);
}

// Convert Starshatter Matrix -> Unreal FMatrix (row-major in this usage).
// This keeps Sprite.cpp using Unreal math operations (FMatrix/FVector/FQuat),
// without requiring global type changes to Camera/Matrix yet.
static FMatrix ToFMatrix(const Matrix& M)
{
	FMatrix Out = FMatrix::Identity;

	// Starshatter Matrix is accessed as M(r,c) in existing code.
	// Unreal FMatrix is indexed as M[row][col].
	Out.M[0][0] = (float)M(0, 0);  Out.M[0][1] = (float)M(0, 1);  Out.M[0][2] = (float)M(0, 2);  Out.M[0][3] = 0.0f;
	Out.M[1][0] = (float)M(1, 0);  Out.M[1][1] = (float)M(1, 1);  Out.M[1][2] = (float)M(1, 2);  Out.M[1][3] = 0.0f;
	Out.M[2][0] = (float)M(2, 0);  Out.M[2][1] = (float)M(2, 1);  Out.M[2][2] = (float)M(2, 2);  Out.M[2][3] = 0.0f;
	Out.M[3][0] = 0.0f;            Out.M[3][1] = 0.0f;            Out.M[3][2] = 0.0f;            Out.M[3][3] = 1.0f;

	return Out;
}

static FVector RowAsVector(const FMatrix& M, int Row)
{
	return FVector(M.M[Row][0], M.M[Row][1], M.M[Row][2]);
}

// +--------------------------------------------------------------------+

Sprite::Sprite()
	: w(0), h(0), nframes(0), own_frames(0),
	frames(0), frame_index(0), frame_time(100), loop(0), shade(1.0),
	angle(0.0), blend_mode(4), filter(1), vset(4), poly(0)
{
	trans = true;

	vset.space = VertexSet::WORLD_SPACE;
	for (int i = 0; i < 4; i++) {
		vset.diffuse[i] = Color::White.Value();
	}

	vset.tu[0] = 0.0f;
	vset.tv[0] = 0.0f;
	vset.tu[1] = 1.0f;
	vset.tv[1] = 0.0f;
	vset.tu[2] = 1.0f;
	vset.tv[2] = 1.0f;
	vset.tu[3] = 0.0f;
	vset.tv[3] = 1.0f;

	poly.nverts = 4;
	poly.vertex_set = &vset;
	poly.material = &mtl;
	poly.verts[0] = 0;
	poly.verts[1] = 1;
	poly.verts[2] = 2;
	poly.verts[3] = 3;
}

// +--------------------------------------------------------------------+

Sprite::Sprite(Bitmap* animation, int length, int repeat, int share)
	: w(0), h(0), nframes(0), own_frames(0),
	frames(0), frame_index(0), frame_time(67), loop(0), shade(1.0),
	angle(0.0), blend_mode(4), filter(1), vset(4), poly(0)
{
	trans = true;
	SetAnimation(animation, length, repeat, share);

	vset.space = VertexSet::WORLD_SPACE;
	for (int i = 0; i < 4; i++) {
		vset.diffuse[i] = Color::White.Value();
	}

	vset.tu[0] = 0.0f;
	vset.tv[0] = 0.0f;
	vset.tu[1] = 1.0f;
	vset.tv[1] = 0.0f;
	vset.tu[2] = 1.0f;
	vset.tv[2] = 1.0f;
	vset.tu[3] = 0.0f;
	vset.tv[3] = 1.0f;

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
	if (own_frames) {
		if (nframes == 1)
			delete frames;
		else
			delete[] frames;
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
Sprite::SetAnimation(Bitmap* animation, int length, int repeat, int share)
{
	if (animation) {
		strncpy_s(name, animation->GetFilename(), 31);
		name[31] = 0;

		if (own_frames) {
			if (nframes == 1)
				delete frames;
			else
				delete[] frames;
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
	else {
		UE_LOG(LogStarshatterWars, Warning, TEXT("Sprite::SetAnimation called with null animation"));
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
		vset.tu[0] = 0.0f;
		vset.tv[0] = 0.0f;
		vset.tu[1] = 1.0f;
		vset.tv[1] = 0.0f;
		vset.tu[2] = 1.0f;
		vset.tv[2] = 1.0f;
		vset.tu[3] = 0.0f;
		vset.tv[3] = 1.0f;
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

Bitmap*
Sprite::Frame() const
{
	return frames + frame_index;
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

		// Convert Starshatter orientation matrix to Unreal FMatrix:
		const FMatrix CamM = ToFMatrix(camera->Orientation());

		// Camera basis (matches original usage of orient(0,*) and orient(1,*)):
		FVector Right = RowAsVector(CamM, 0);
		FVector Up = RowAsVector(CamM, 1);

		// Camera normal: original code used nrm = camera->vpn() * -1
		const Vec3 vpn_ss = camera->vpn();
		const FVector Nrm = FVector((float)vpn_ss.x, (float)vpn_ss.y, (float)vpn_ss.z) * -1.0f;

		// Apply roll (angle is assumed radians, matching Render2D's cos/sin usage):
		if (FMath::Abs((float)angle) > KINDA_SMALL_NUMBER) {
			const FQuat RollQ(Nrm.GetSafeNormal(), (float)angle);
			Right = RollQ.RotateVector(Right);
			Up = RollQ.RotateVector(Up);
		}

		// Vertex diffuse as packed ARGB using FColor:
		const uint8  c = ClampColorByte(shade);
		const FColor White(c, c, c, c);
		const DWORD  diff = (DWORD)White.ToPackedARGB();

		// Material still uses Starshatter ColorValue/Color types:
		const ColorValue white((float)shade, (float)shade, (float)shade, (float)shade);

		// Scale axes to sprite half extents:
		const FVector vx = Right * (float)(w / 2.0f);
		const FVector vy = Up * (float)(h / 2.0f);

		// loc assumed FVector in the UE port:
		vset.loc[0] = loc - vx + vy;
		vset.nrm[0] = Nrm;
		vset.diffuse[0] = diff;

		vset.loc[1] = loc + vx + vy;
		vset.nrm[1] = Nrm;
		vset.diffuse[1] = diff;

		vset.loc[2] = loc + vx - vy;
		vset.nrm[2] = Nrm;
		vset.diffuse[2] = diff;

		vset.loc[3] = loc - vx - vy;
		vset.nrm[3] = Nrm;
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

	const uint8  c = ClampColorByte(shade);
	const FColor White(c, c, c, c);
	const DWORD  diff = (DWORD)White.ToPackedARGB();

	const ColorValue white((float)shade, (float)shade, (float)shade, (float)shade);

	const double ca = std::cos(Angle());
	const double sa = std::sin(Angle());

	const double w2 = Width() / 2.0;
	const double h2 = Height() / 2.0;

	// loc assumed FVector:
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
