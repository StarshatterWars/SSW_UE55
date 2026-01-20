/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright © 2025-2026. All Rights Reserved.

	ORIGINAL AUTHOR AND STUDIO: John DiCamillo / Destroyer Studios LLC

	SUBSYSTEM:    Stars.exe
	FILE:         DriveSprite.cpp
	AUTHOR:       Carlos Bott


	OVERVIEW
	========
	Sprite for rendering drive flares.  Remains visible at extreme ranges.
*/

#include "DriveSprite.h"

#include "Math/Vector.h"
#include "Math/UnrealMathUtility.h"
#include "Logging/LogMacros.h"

#include "Camera.h"
#include "SimScene.h"
#include "Video.h"

#ifndef STARSHATTERWARS_LOG_DEFINED
#define STARSHATTERWARS_LOG_DEFINED
DECLARE_LOG_CATEGORY_EXTERN(LogStarshatterWars, Log, All);
#endif

// +--------------------------------------------------------------------+

DriveSprite::DriveSprite()
	: Sprite()
	, glow(nullptr)
	, effective_radius(0)
	, front(FVector::ZeroVector)
	, bias(0)
{
	luminous = true;
}

DriveSprite::DriveSprite(UTexture2D* animation, UTexture2D* g)
	: Sprite(animation)
	, glow(g)
	, effective_radius(0)
	, front(FVector::ZeroVector)
	, bias(0)
{
	luminous = true;
}

DriveSprite::DriveSprite(UTexture2D* animation, int length, int repeat, int share)
	: Sprite(animation, length, repeat, share)
	, glow(nullptr)
	, effective_radius(0)
	, front(FVector::ZeroVector)
	, bias(0)
{
	luminous = true;
}

DriveSprite::~DriveSprite()
{
}

// +--------------------------------------------------------------------+

void
DriveSprite::SetFront(const FVector& f)
{
	front = f;
	front.Normalize();
}

void
DriveSprite::SetBias(DWORD b)
{
	bias = b;
}

// +--------------------------------------------------------------------+

void
DriveSprite::Render(Video* video, DWORD flags)
{
	if (!video || ((flags & RENDER_ADDITIVE) == 0))
		return;

	if (shade > 0 && !hidden && (life > 0 || loop)) {
		const Camera* cam = video->GetCamera();
		bool          z_disable = false;

		if (bias)
			video->SetRenderState(Video::Z_BIAS, bias);

		if (!front.IsNearlyZero()) {
			FVector test = loc;

			if (scene && cam) {
				FVector dir = front;

				double intensity = cam->vpn() * dir * -1;
				double distance = FVector(cam->Pos() - test).Length();

				if (intensity > 0.05) {
					if (!scene->IsLightObscured(cam->Pos(), test, 8)) {
						video->SetRenderState(Video::Z_ENABLE, false);
						z_disable = true;

						if (glow) {
							intensity = FMath::Pow(intensity, 3.0);

							if (distance > 5e3)
								intensity *= (1 - (distance - 5e3) / 45e3);

							if (intensity > 0) {
								UTexture2D* tmp_frame = frames;
								double      tmp_shade = shade;
								int         tmp_w = w;
								int         tmp_h = h;

								if (glow->Width() != frames->Width()) {
									double wscale = (double)glow->Width() / (double)frames->Width();
									double hscale = (double)glow->Height() / (double)frames->Height();

									w = (int)(w * wscale);
									h = (int)(h * hscale);
								}

								shade = intensity;
								frames = glow;

								Sprite::Render(video, flags);

								frames = tmp_frame;
								shade = tmp_shade;
								w = tmp_w;
								h = tmp_h;
							}
						}
					}
				}
			}
		}

		if (effective_radius - radius > 0.1) {
			double scale_up = effective_radius / radius;
			int    tmp_w = w;
			int    tmp_h = h;

			w = (int)(w * scale_up);
			h = (int)(h * scale_up);

			Sprite::Render(video, flags);

			w = tmp_w;
			h = tmp_h;
		}

		else {
			Sprite::Render(video, flags);
		}

		if (bias)
			video->SetRenderState(Video::Z_BIAS, 0);

		if (z_disable)
			video->SetRenderState(Video::Z_ENABLE, true);
	}
}

