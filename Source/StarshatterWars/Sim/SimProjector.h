/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright (C) 2025-2026. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         SimProjector.h
	AUTHOR:       Carlos Bott
	ORIGINAL:     John DiCamillo / Destroyer Studios LLC

	OVERVIEW
	========
	3D Projection Camera class
*/

// Starshatter core:
#include "Geometry.h"
#include "Polygon.h"

// Minimal Unreal includes (FVector):
#include "Math/Vector.h"

// +--------------------------------------------------------------------+

class Window;
class Camera;

// +--------------------------------------------------------------------+

class SimProjector
{
public:
	SimProjector(Window* InWindow, Camera* InCamera);
	virtual ~SimProjector();

	// Operations:
	virtual void   UseWindow(Window* win);
	virtual void   UseCamera(Camera* cam);
	virtual void   SetDepthScale(float scale);
	virtual double GetDepthScale() const;
	virtual void   SetFieldOfView(double fov);
	virtual double GetFieldOfView() const;
	virtual int    SetInfinite(int i);
	virtual void   StartFrame();

	// Accessors:
	FVector        Pos() const;
	FVector        vrt() const;
	FVector        vup() const;
	FVector        vpn() const;
	const Matrix& Orientation() const;

	double         XAngle() const { return xangle; }
	double         YAngle() const { return yangle; }

	bool           IsOrthogonal()    const { return orthogonal; }
	void           SetOrthogonal(bool o) { orthogonal = o; }

	// Projection and clipping geometry:
	virtual void   Transform(FVector& vec)    const;

	virtual void   Project(FVector& vec, bool clamp = true) const;
	virtual void   ProjectRect(FVector& origin, double& w, double& h) const;

	virtual float  ProjectRadius(const FVector& vec, float radius) const;

	virtual void   Unproject(FVector& point) const;
	int            IsVisible(const FVector& v, float radius) const;
	int            IsBoxVisible(const FVector* p) const;

	float          ApparentRadius(const FVector& v, float radius) const;

	virtual void   SetWorldSpace() { frustum_planes = world_planes; }
	virtual void   SetViewSpace() { frustum_planes = view_planes; }

	Plane* GetCurrentClipPlanes() { return frustum_planes; }

	void           SetUpFrustum();
	void           ViewToWorld(FVector& pin, FVector& pout);
	void           SetWorldspaceClipPlane(FVector& normal, Plane& plane);

protected:
	Camera* camera;

	int            width, height;
	double         field_of_view;
	double         xscreenscale, yscreenscale, maxscale;
	double         xcenter, ycenter;
	double         xangle, yangle;

	int            infinite;
	float          depth_scale;
	bool           orthogonal;

	enum DISPLAY_CONST {
		NUM_FRUSTUM_PLANES = 4,
	};

	Plane* frustum_planes;
	Plane          world_planes[NUM_FRUSTUM_PLANES];
	Plane          view_planes[NUM_FRUSTUM_PLANES];

	float          xclip0, xclip1;
	float          yclip0, yclip1;
};

