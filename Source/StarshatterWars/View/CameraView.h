/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright (c) 2025-2026. All Rights Reserved.

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

#include "Types.h"
#include "View.h"
#include "Camera.h"
#include "SimProjector.h"
#include "Video.h"
#include "List.h"
#include "Bitmap.h"

#include "Math/Vector.h" // FVector

// +--------------------------------------------------------------------+

class UTexture2D;

class Video;
class SimScene;
class Camera;
class Graphic;

// +--------------------------------------------------------------------+

class CameraView : public View
{
public:
	static const char* TYPENAME() { return "CameraView"; }

	CameraView(Window* c, Camera* cam, SimScene* s);
	virtual ~CameraView();

	// Operations:
	virtual void	Refresh();
	virtual void	OnWindowMove();
	virtual void	UseCamera(Camera* cam);
	virtual void	UseScene(SimScene* scene);
	virtual void	LensFlare(int on, double dim = 1);
	virtual void	LensFlareElements(Bitmap* halo, Bitmap* e1, Bitmap* e2, Bitmap* e3);
	virtual void	SetDepthScale(float scale);

	// accessors:
	Camera*			GetCamera()                         const { return camera; }
	SimProjector*	GetProjector() { return &projector; }
	SimScene*		GetScene()                          const { return scene; }
	virtual void	SetFieldOfView(double fov);
	virtual double	GetFieldOfView()                    const;
	virtual void	SetProjectionType(DWORD pt);
	virtual DWORD	GetProjectionType()                 const;

	FVector        Pos()                               const { return (FVector)camera->Pos(); }
	FVector        vrt() { return (FVector)camera->vrt(); }
	FVector        vup() { return (FVector)camera->vup(); }
	FVector        vpn() { return (FVector)camera->vpn(); }
	const Matrix& Orientation()                       const { return camera->Orientation(); }

	FVector        SceneOffset()                       const { return camera_loc; }

	// projection and clipping geometry:
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
	Camera*			camera;
	SimScene*		scene;
	Video*			video;

	virtual void   WorldPlaneToView(Plane& plane);

	FVector        camera_loc;
	FVector        cvrt;
	FVector        cvup;
	FVector        cvpn;

	SimProjector   projector;
	int            infinite;
	int            width;
	int            height;
	DWORD          projection_type;

	// lens flare:
	int            lens_flare_enable;
	double         lens_flare_dim;
	Bitmap*		    halo_bitmap;
	Bitmap*		    elem_bitmap[3];

	List<Graphic>  graphics;
};
