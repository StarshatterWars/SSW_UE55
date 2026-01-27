/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    nGenEx.lib
    FILE:         CameraView.h
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR: John DiCamillo
    ORIGINAL STUDIO: Destroyer Studios LLC

    OVERVIEW
    ========
    3D Projection Camera View class
*/

#pragma once

#include "CoreMinimal.h"

// Base is now UView (a UUserWidget):
#include "View.h"

// Legacy:
#include "Types.h"
#include "Camera.h"
#include "SimProjector.h"
#include "Video.h"
#include "List.h"
#include "Bitmap.h"

// Unreal:
#include "Math/Vector.h"
#include "GameStructs.h"

#include "CameraView.generated.h"

class Video;
class SimScene;
class Camera;
class Graphic;
class Window;

// +--------------------------------------------------------------------+

UCLASS()
class STARSHATTERWARS_API UCameraView : public UView
{
    GENERATED_BODY()

public:
    UCameraView(const FObjectInitializer& ObjectInitializer);

    static const char* TYPENAME() { return "CameraView"; }

    // ----------------------------------------------------------------
    // Unreal-friendly init (replaces legacy constructor params)
    // ----------------------------------------------------------------
    void InitializeView(Window* InWindow, Camera* InCamera, SimScene* InScene);

    // Operations:
    virtual void   Refresh() override;
    virtual void   OnWindowMove() override;
    virtual void   OnShow() override {}
    virtual void   OnHide() override {}

    virtual void   UseCamera(Camera* Cam);
    virtual void   UseScene(SimScene* Scene);
    virtual void   LensFlare(int On, double Dim = 1.0);
    virtual void   LensFlareElements(Bitmap* Halo, Bitmap* E1, Bitmap* E2, Bitmap* E3);
    virtual void   SetDepthScale(float Scale);

    // Accessors:
    Camera* GetCamera() const { return camera; }
    SimProjector* GetProjector() { return &projector; }
    SimScene* GetScene() const { return scene; }

    virtual void   SetFieldOfView(double fov);
    virtual double GetFieldOfView() const;

    virtual void   SetProjectionType(DWORD pt);
    virtual DWORD  GetProjectionType() const;

    FVector        Pos() const { return camera ? (FVector)camera->Pos() : FVector::ZeroVector; }
    FVector        vrt() { return camera ? (FVector)camera->vrt() : FVector::ZeroVector; }
    FVector        vup() { return camera ? (FVector)camera->vup() : FVector::ZeroVector; }
    FVector        vpn() { return camera ? (FVector)camera->vpn() : FVector::ZeroVector; }

    const Matrix& Orientation() const { return camera->Orientation(); }

    FVector        SceneOffset() const { return camera_loc; }

    // Projection and clipping geometry:
    virtual void   TranslateScene();
    virtual void   UnTranslateScene();
    virtual void   MarkVisibleObjects();
    virtual void   MarkVisibleLights(Graphic* g, DWORD flags);

    virtual void   RenderScene();
    virtual void   RenderSceneObjects(bool distant = false);
    virtual void   RenderForeground();
    virtual void   RenderBackground();
    virtual void   RenderSprites();
    virtual void   RenderLensFlare();
    virtual void   Render(Graphic* g, DWORD flags);

    virtual void   FindDepth(Graphic* g);
    virtual int    SetInfinite(int i);

protected:
    virtual void   WorldPlaneToView(Plane& plane);

private:
    // Legacy pointers (raw per your direction):
    Camera* camera = nullptr;
    SimScene* scene = nullptr;
    Video* video = nullptr;

    // NOTE: legacy Window is still used by the nGenEx pipeline for DrawBitmap.
    // UView may or may not store this; we keep a local copy to be explicit.
    Window* window = nullptr;

    FVector     camera_loc = FVector::ZeroVector;
    FVector     cvrt = FVector::ZeroVector;
    FVector     cvup = FVector::ZeroVector;
    FVector     cvpn = FVector::ZeroVector;

    SimProjector projector;

    int         infinite = 0;
    int         width = 0;
    int         height = 0;
    DWORD       projection_type = Video::PROJECTION_PERSPECTIVE;

    // Lens flare:
    int         lens_flare_enable = 0;
    double      lens_flare_dim = 0.0;
    Bitmap* halo_bitmap = nullptr;
    Bitmap* elem_bitmap[3] = { nullptr, nullptr, nullptr };

    List<Graphic> graphics;
};
