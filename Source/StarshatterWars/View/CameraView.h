/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (C) 2025–2026.

    SUBSYSTEM:    nGenEx.lib (ported to Unreal)
    FILE:         CameraView.h
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo
    Destroyer Studios LLC
    Copyright (C) 1997–2004.

    OVERVIEW
    ========
    CameraView
    ----------
    Legacy 3D projection camera view.

    CameraView is a non-UObject view that configures the Video/Projector
    pipeline to render a Scene from a given Camera into the view rectangle.
    Rendering remains delegated to the Video layer; this class coordinates
    visibility, sorting, projection, and optional lens flare.
*/

#pragma once

#include "CoreMinimal.h"

#include "View.h"
#include "Camera.h"
#include "SimProjector.h"
#include "Video.h"
#include "List.h"


// Forward declarations (keep header light):
class Screen;
class SimScene;
class Bitmap;
class Graphic;

class CameraView : public View
{
public:
    static const char* TYPENAME() { return "CameraView"; }

    // Root-style ctor: you now build Views using Screen + rect
    CameraView(Screen* InScreen, int ax, int ay, int aw, int ah, Camera* cam, SimScene* s);
    virtual ~CameraView() override;

    // Operations:
    virtual void   Refresh() override;
    virtual void   OnWindowMove() override;

    virtual void   UseCamera(Camera* cam);
    virtual void   UseScene(SimScene* scene);

    virtual void   LensFlareElements(Bitmap* halo, Bitmap* e1 = nullptr, Bitmap* e2 = nullptr, Bitmap* e3 = nullptr);
    virtual void   LensFlare(int on, double dim = 1);
    virtual void   SetDepthScale(float scale);

    // Accessors:
    Camera* GetCamera()                   const { return camera; }
    SimProjector* GetProjector()        { return &projector; }


    SimScene* GetScene()                  const { return scene; }

    virtual void   SetFieldOfView(double fov);
    virtual double GetFieldOfView()              const;

    virtual void   SetProjectionType(uint32 pt);
    virtual uint32 GetProjectionType()           const;

    // Convenience pass-throughs (legacy naming):
    FVector        Pos() const { return camera->Pos(); }
    FVector        vrt() { return camera->vrt(); }
    FVector        vup() { return camera->vup(); }
    FVector        vpn() { return camera->vpn(); }

    const Matrix& Orientation() const { return camera->Orientation(); }
    FVector          SceneOffset()  const { return camera_loc; }

    // Projection / clipping geometry:
    virtual void   TranslateScene();
    virtual void   UnTranslateScene();
    virtual void   MarkVisibleObjects();
    virtual void   MarkVisibleLights(Graphic* g, uint32 flags);

    virtual void   RenderScene();
    virtual void   RenderSceneObjects(bool distant = false);
    virtual void   RenderForeground();
    virtual void   RenderBackground();
    virtual void   RenderSprites();
    virtual void   RenderLensFlare();
    virtual void   Render(Graphic* g, uint32 flags);

    virtual void   FindDepth(Graphic* g);
    virtual int    SetInfinite(int i);

protected:
    virtual void   WorldPlaneToView(Plane& plane);

protected:
    Camera* camera = nullptr;
    SimScene* scene = nullptr;
    Video* video = nullptr;

    Point          camera_loc;
    Vec3           cvrt;
    Vec3           cvup;
    Vec3           cvpn;

    SimProjector      projector;

    int            infinite = 0;
    int            width = 0;
    int            height = 0;
    uint32         projection_type = 0;

    // Lens flare:
    int            lens_flare_enable = 0;
    double         lens_flare_dim = 0.0;
    Bitmap* halo_bitmap = nullptr;
    Bitmap* elem_bitmap[3] = { nullptr, nullptr, nullptr };

    // Visible scene list:
    List<Graphic>  graphics;
};
