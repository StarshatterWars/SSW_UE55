/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         Bolt.cpp
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO:
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    3D Bolt (Polygon) Object
*/

#include "Bolt.h"

#include "Camera.h"
#include "Video.h"

#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogBolt, Log, All);

// +--------------------------------------------------------------------+

Bolt::Bolt(double len, double wid, UTexture2D* tex, int share)
    : vset(4),
    poly(0),
    texture(tex),
    length(len),
    width(wid),
    shade(1.0),
    vpn(0.0f, 1.0f, 0.0f),
    shared(share)
{
    trans = true;

    // Vec3/Point -> FVector
    loc = FVector(0.0f, 0.0f, 1000.0f);

    vset.nverts = 4;

    vset.loc[0] = FVector((float)width, 0.0f, 1000.0f);
    vset.loc[1] = FVector((float)width, (float)-length, 1000.0f);
    vset.loc[2] = FVector((float)-width, (float)-length, 1000.0f);
    vset.loc[3] = FVector((float)-width, 0.0f, 1000.0f);

    vset.tu[0] = 0.0f;
    vset.tv[0] = 0.0f;
    vset.tu[1] = 1.0f;
    vset.tv[1] = 0.0f;
    vset.tu[2] = 1.0f;
    vset.tv[2] = 1.0f;
    vset.tu[3] = 0.0f;
    vset.tv[3] = 1.0f;

    Plane plane(vset.loc[0], vset.loc[1], vset.loc[2]);

    for (int i = 0; i < 4; i++) {
        vset.nrm[i] = plane.normal;
    }

    mtl.Ka = Color::White;
    mtl.Kd = Color::White;
    mtl.Ks = Color::Black;
    mtl.Ke = Color::White;

    // Bitmap -> UTexture2D*
    mtl.tex_diffuse = texture;
    mtl.tex_emissive = texture;

    mtl.blend = Video::BLEND_ADDITIVE;

    poly.nverts = 4;
    poly.vertex_set = &vset;
    poly.material = &mtl;
    poly.verts[0] = 0;
    poly.verts[1] = 1;
    poly.verts[2] = 2;
    poly.verts[3] = 3;

    radius = (float)((length > width) ? (length) : (width * 2));

    // Original: copied texture filename into name[] from Bitmap.
    // With UTexture2D*, we cannot assume a filename accessor exists here.
    // Leave name unchanged and log at verbose level if a texture is provided.
    if (texture) {
        UE_LOG(LogBolt, VeryVerbose, TEXT("Bolt created with UTexture2D* texture (name[] not auto-derived)."));
    }
}

// +--------------------------------------------------------------------+

Bolt::~Bolt()
{
}

// +--------------------------------------------------------------------+

void
Bolt::Render(Video* video, DWORD flags)
{
    if ((flags & RENDER_ADDITIVE) == 0)
        return;

    if (visible && !hidden && video && life) {
        const Camera* camera = video->GetCamera();

        const FVector head = loc;
        const FVector tail = origin;
        const FVector vtail = tail - head;
        const FVector vcam = camera->Pos() - loc;

        // Build lateral vector perpendicular to camera->bolt plane:
        FVector vtmp = FVector::CrossProduct(vcam, vtail);
        vtmp.Normalize();

        const FVector vlat = vtmp * (float)(-width);
        const FVector vnrm = camera->vpn() * -1.0f;

        vset.loc[0] = head + vlat;
        vset.loc[1] = tail + vlat;
        vset.loc[2] = tail - vlat;
        vset.loc[3] = head - vlat;

        vset.nrm[0] = vnrm;
        vset.nrm[1] = vnrm;
        vset.nrm[2] = vnrm;
        vset.nrm[3] = vnrm;

        ColorValue white((float)shade, (float)shade, (float)shade);
        mtl.Ka = white;
        mtl.Kd = white;
        mtl.Ks = Color::Black;
        mtl.Ke = white;

        video->DrawPolys(1, &poly);
    }
}

// +--------------------------------------------------------------------+

void
Bolt::Update()
{
}

// +--------------------------------------------------------------------+

void
Bolt::TranslateBy(const FVector& ref)
{
    loc = loc - ref;
    origin = origin - ref;
}

// +--------------------------------------------------------------------+

void
Bolt::SetOrientation(const Matrix& o)
{
    vpn = FVector((float)o(2, 0), (float)o(2, 1), (float)o(2, 2));
    origin = loc + (vpn * (float)(-length));
}

void
Bolt::SetDirection(const FVector& v)
{
    vpn = v;
    origin = loc + (vpn * (float)(-length));
}

void
Bolt::SetEndPoints(const FVector& from, const FVector& to)
{
    loc = to;
    origin = from;

    vpn = to - from;

    // Original code relied on Point::Normalize() returning the original vector length.
    const float Len = vpn.Length();
    if (Len > 0.0f) {
        vpn /= Len;
    }

    length = (double)Len;
    radius = Len;
}

void
Bolt::SetTextureOffset(double from, double to)
{
    vset.tu[0] = (float)from;
    vset.tu[1] = (float)to;
    vset.tu[2] = (float)to;
    vset.tu[3] = (float)from;
}
