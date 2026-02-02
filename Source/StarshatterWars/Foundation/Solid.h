/*  Starshatter Wars
    Fractal Dev Studios
    Copyright (C) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO:
    John DiCamillo, Destroyer Studios LLC

    SUBSYSTEM:    nGenEx.lib
    FILE:         Solid.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Solid mesh renderable (owns/uses a Model)
*/

#pragma once

#include "Graphic.h"
#include "Video.h"   // for Video, DWORD (keep for now)
#include "List.h"

// Minimal Unreal includes:
#include "Math/Vector.h"
#include "Math/Matrix.h"

// Forward declarations:
class Model;
class Shadow;
class SimLight;
class SimProjector;
class Poly;
class UTexture2D;

class Solid : public Graphic
{
public:
    static const char* TYPENAME() { return "Solid"; }
    enum { NAMELEN = 64 };

    static bool IsCollisionEnabled();
    static void EnableCollision(bool enable);

    Solid();
    virtual ~Solid();

    // operations
    virtual void Render(Video* video, DWORD flags);
    virtual void SelectDetail(SimProjector* p);
    virtual void ProjectScreenRect(SimProjector* p);
    virtual void Update();

    // accessors / mutators
    Model* GetModel() const { return model; }

    void   GetAllTextures(List<UTexture2D*>& textures);

    virtual bool IsDynamic() const;
    virtual void SetDynamic(bool d);
    virtual void SetLuminous(bool l);

    virtual void SetOrientation(const FMatrix& o);
    virtual void SetOrientation(const Solid& match);

    const FMatrix& Orientation() const { return OrientationMatrix; }

    float Roll()  const { return roll; }
    float Pitch() const { return pitch; }
    float Yaw()   const { return yaw; }

    virtual bool IsSolid() const { return true; }

    // stencil shadows
    virtual void CreateShadows(int nlights = 1);
    virtual void UpdateShadows(List<SimLight>& lights);
    List<Shadow>& GetShadows() { return shadows; }

    // model management
    bool Load(const char* mag_file, double scale = 1.0);
    bool Load(class ModelFile* loader, double scale = 1.0);
    void UseModel(Model* model);
    void ClearModel();
    bool Rescale(double scale);

    // collision / picking
    virtual int  CollidesWith(Graphic& o);
    virtual int  CheckRayIntersection(FVector pt, FVector vpn, double len, FVector& ipt,
        bool treat_translucent_polys_as_solid = true);
    virtual Poly* GetIntersectionPoly() const { return intersection_poly; }

    // buffer management
    virtual void DeletePrivateData();
    virtual void InvalidateSurfaceData();
    virtual void InvalidateSegmentData();

protected:
    Model* model = nullptr;
    bool   own_model = true;

    float  roll = 0.0f;
    float  pitch = 0.0f;
    float  yaw = 0.0f;

    FMatrix OrientationMatrix;

    Poly* intersection_poly = nullptr;

    List<Shadow> shadows;
};
