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

    if (shade <= 0 || hidden || (life <= 0 && !loop))
        return;

    const Camera* cam = video->GetCamera();
    bool z_disabled = false;

    // Logical depth bias (renderer-specific handling inside Video)
    if (bias)
        video->SetDepthBias(bias);

    // ----------------------------
    // Forward-facing glow test
    // ----------------------------
    if (!front.IsNearlyZero() && cam && scene) {
        FVector test = loc;
        FVector dir = front;

        double intensity =
            FVector::DotProduct(cam->vpn(), dir) * -1.0;

        double distance =
            FVector(cam->Pos() - test).Length();

        if (intensity > 0.05) {
            if (!scene->IsLightObscured(cam->Pos(), test, 8)) {

                video->DisableDepthTest();
                z_disabled = true;

                if (glow) {
                    intensity = FMath::Pow(intensity, 3.0);

                    if (distance > 5000.0)
                        intensity *= (1.0 - (distance - 5000.0) / 45000.0);

                    if (intensity > 0) {
                        // Preserve original state
                        UTexture2D* saved_frames = frames;
                        double      saved_shade = shade;
                        int         saved_w = w;
                        int         saved_h = h;

                        // Scale sprite to glow texture
                        if (glow->GetSizeX() != frames->GetSizeX()) {
                            double wscale =
                                (double)glow->GetSizeX() /
                                (double)frames->GetSizeX();

                            double hscale =
                                (double)glow->GetSizeY() /
                                (double)frames->GetSizeY();

                            w = (int)(w * wscale);
                            h = (int)(h * hscale);
                        }

                        shade = intensity;
                        frames = glow;

                        Sprite::Render(video, flags);

                        // Restore state
                        frames = saved_frames;
                        shade = saved_shade;
                        w = saved_w;
                        h = saved_h;
                    }
                }
            }
        }
    }

    // ----------------------------
    // Radius scaling (engine plume)
    // ----------------------------
    if (effective_radius - radius > 0.1) {
        double scale = effective_radius / radius;

        int saved_w = w;
        int saved_h = h;

        w = (int)(w * scale);
        h = (int)(h * scale);

        Sprite::Render(video, flags);

        w = saved_w;
        h = saved_h;
    }
    else {
        Sprite::Render(video, flags);
    }

    // Restore render state
    if (bias)
        video->SetDepthBias(0);

    if (z_disabled)
        video->EnableDepthTest();
}


