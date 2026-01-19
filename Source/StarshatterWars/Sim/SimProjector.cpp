/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright (C) 2025-2026. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         SimProjector.cpp
	AUTHOR:       Carlos Bott
	ORIGINAL:     John DiCamillo / Destroyer Studios LLC

	OVERVIEW
	========
	3D Projection Camera class
*/

#include "SimProjector.h"

// Starshatter core:
#include "Window.h"
#include "Camera.h"

// Minimal Unreal includes:
#include "Math/Vector.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogStarshatterSimProjector, Log, All);

// +--------------------------------------------------------------------+

static const float   CLIP_PLANE_EPSILON = 0.0001f;
static const double  Z_NEAR = 1.0;

// +--------------------------------------------------------------------+

static Camera  emergency_cam;

// +--------------------------------------------------------------------+

SimProjector::SimProjector(Window* window, Camera* cam)
	: camera(cam), infinite(0), depth_scale(1.0f), orthogonal(false), field_of_view(2)
{
	if (!camera)
		camera = &emergency_cam;

	UseWindow(window);
}

SimProjector::~SimProjector()
{
}

// +--------------------------------------------------------------------+

void
SimProjector::UseCamera(Camera* cam)
{
	if (cam)
		camera = cam;
	else
		camera = &emergency_cam;
}

void
SimProjector::UseWindow(Window* win)
{
	Rect r = win->GetRect();
	width = r.w;
	height = r.h;

	xcenter = (width / 2.0);
	ycenter = (height / 2.0);

	xclip0 = 0.0f;
	xclip1 = (float)width - 0.5f;
	yclip0 = 0.0f;
	yclip1 = (float)height - 0.5f;

	SetFieldOfView(field_of_view);
}

void SimProjector::SetFieldOfView(double fov)
{
	field_of_view = fov;

	xscreenscale = width / fov;
	yscreenscale = height / fov;

	maxscale = FMath::Max(xscreenscale, yscreenscale);

	xangle = atan(2.0 / fov * maxscale / xscreenscale);
	yangle = atan(2.0 / fov * maxscale / yscreenscale);
}

double
SimProjector::GetFieldOfView() const
{
	return field_of_view;
}

void
SimProjector::SetDepthScale(float scale)
{
	depth_scale = scale;
}

double
SimProjector::GetDepthScale() const
{
	return depth_scale;
}

int
SimProjector::SetInfinite(int i)
{
	int old = infinite;
	infinite = i;
	return old;
}

// +--------------------------------------------------------------------+

void
SimProjector::StartFrame()
{
	SetUpFrustum();
	SetWorldSpace();
}

// +--------------------------------------------------------------------+
// Accessors that formerly exposed Point/Vec3:
// +--------------------------------------------------------------------+

FVector
SimProjector::Pos() const
{
	const auto P = camera->Pos();
	return FVector((float)P.X, (float)P.Y, (float)P.Z);
}

FVector
SimProjector::vrt() const
{
	const auto V = camera->vrt();
	return FVector((float)V.X, (float)V.Y, (float)V.Z);
}

FVector
SimProjector::vup() const
{
	const auto V = camera->vup();
	return FVector((float)V.X, (float)V.Y, (float)V.Z);
}

FVector
SimProjector::vpn() const
{
	const auto V = camera->vpn();
	return FVector((float)V.X, (float)V.Y, (float)V.Z);
}

const Matrix&
SimProjector::Orientation() const
{
	return camera->Orientation();
}

// +--------------------------------------------------------------------+
// Transform a point from worldspace to viewspace.
// +--------------------------------------------------------------------+

void
SimProjector::Transform(FVector& vec) const
{
	FVector tvert = vec;

	// Translate into a viewpoint-relative coordinate
	if (!infinite) {
		const FVector CamPos = Pos();
		tvert -= CamPos;
	}

	// old method (dot against camera basis vectors):
	const FVector Vrt = vrt();
	const FVector Vup = vup();
	const FVector Vpn = vpn();

	vec.X = FVector::DotProduct(tvert, Vrt);
	vec.Y = FVector::DotProduct(tvert, Vup);
	vec.Z = FVector::DotProduct(tvert, Vpn);
}

// +--------------------------------------------------------------------+
// APPARENT RADIUS OF PROJECTED OBJECT
// +--------------------------------------------------------------------+

float
SimProjector::ProjectRadius(const FVector& v, float radius) const
{
	return (float)fabs((radius * maxscale) / v.Z);
}

// +--------------------------------------------------------------------+
// IN PLACE PROJECTION OF POINT
// +--------------------------------------------------------------------+

void
SimProjector::Project(FVector& v, bool clamp) const
{
	double zrecip;

	if (orthogonal) {
		const double scale = field_of_view / 2;
		v.X = (float)(xcenter + scale * v.X);
		v.Y = (float)(height - (ycenter + scale * v.Y));
		v.Z = 0.0f;
	}
	else {
		zrecip = 2 / v.Z;
		v.X = (float)(xcenter + maxscale * v.X * zrecip);
		v.Y = (float)(height - (ycenter + maxscale * v.Y * zrecip));
		v.Z = (float)(1 - zrecip);
	}

	// clamp the point to the viewport:
	if (clamp) {
		if (v.X < xclip0) v.X = xclip0;
		if (v.X > xclip1) v.X = xclip1;
		if (v.Y < yclip0) v.Y = yclip0;
		if (v.Y > yclip1) v.Y = yclip1;
	}
}

// +--------------------------------------------------------------------+
// IN PLACE UN-PROJECTION OF POINT
// +--------------------------------------------------------------------+

void
SimProjector::Unproject(FVector& v) const
{
	const double zrecip = 1 / v.Z;

	v.X = (float)((v.X - xcenter) / (maxscale * zrecip));
	v.Y = (float)((height - v.Y - ycenter) / (maxscale * zrecip));
}

// +--------------------------------------------------------------------+
// IN PLACE PROJECTION OF RECTANGLE (FOR SPRITES)
// +--------------------------------------------------------------------+

void
SimProjector::ProjectRect(FVector& v, double& w, double& h) const
{
	double zrecip;

	if (orthogonal) {
		const double scale = field_of_view / 2;
		v.X = (float)(xcenter + scale * v.X);
		v.Y = (float)(height - (ycenter + scale * v.Y));
		v.Z = 0.0f;
	}
	else {
		zrecip = 1 / v.Z;
		v.X = (float)(xcenter + 2 * maxscale * v.X * zrecip);
		v.Y = (float)(height - (ycenter + 2 * maxscale * v.Y * zrecip));
		v.Z = (float)(1 - Z_NEAR * zrecip);

		w *= maxscale * zrecip;
		h *= maxscale * zrecip;
	}
}

// +--------------------------------------------------------------------+
// Set up a clip plane with the specified normal.
// +--------------------------------------------------------------------+

void
SimProjector::SetWorldspaceClipPlane(FVector& normal, Plane& plane)
{
	// Rotate the plane normal into worldspace
	FVector worldNormal = FVector::ZeroVector;
	ViewToWorld(normal, worldNormal);

	plane.normal.X = worldNormal.X;
	plane.normal.Y = worldNormal.Y;
	plane.normal.Z = worldNormal.Z;

	const FVector camPos = Pos();
	plane.distance = (float)(camPos.X * plane.normal.X +
		camPos.Y * plane.normal.Y +
		camPos.Z * plane.normal.Z +
		CLIP_PLANE_EPSILON);
}

// +--------------------------------------------------------------------+
// Set up the planes of the frustum, in worldspace coordinates.
// +--------------------------------------------------------------------+

void
SimProjector::SetUpFrustum()
{
	double  angle, s, c;
	FVector normal = FVector::ZeroVector;

	angle = XAngle();
	s = sin(angle);
	c = cos(angle);

	// Left clip plane
	normal.X = (float)s;
	normal.Y = 0.0f;
	normal.Z = (float)c;
	view_planes[0].normal.X = normal.X;
	view_planes[0].normal.Y = normal.Y;
	view_planes[0].normal.Z = normal.Z;
	view_planes[0].distance = CLIP_PLANE_EPSILON;
	SetWorldspaceClipPlane(normal, world_planes[0]);

	// Right clip plane
	normal.X = (float)-s;
	view_planes[1].normal.X = normal.X;
	view_planes[1].normal.Y = normal.Y;
	view_planes[1].normal.Z = normal.Z;
	view_planes[1].distance = CLIP_PLANE_EPSILON;
	SetWorldspaceClipPlane(normal, world_planes[1]);

	angle = YAngle();
	s = sin(angle);
	c = cos(angle);

	// Bottom clip plane
	normal.X = 0.0f;
	normal.Y = (float)s;
	normal.Z = (float)c;
	view_planes[2].normal.X = normal.X;
	view_planes[2].normal.Y = normal.Y;
	view_planes[2].normal.Z = normal.Z;
	view_planes[2].distance = CLIP_PLANE_EPSILON;
	SetWorldspaceClipPlane(normal, world_planes[2]);

	// Top clip plane
	normal.Y = (float)-s;
	view_planes[3].normal.X = normal.X;
	view_planes[3].normal.Y = normal.Y;
	view_planes[3].normal.Z = normal.Z;
	view_planes[3].distance = CLIP_PLANE_EPSILON;
	SetWorldspaceClipPlane(normal, world_planes[3]);

	frustum_planes = world_planes;
}

// +--------------------------------------------------------------------+
// Clip the point against the frustum and return 1 if partially inside
// Return 2 if completely inside
// +--------------------------------------------------------------------+

int
SimProjector::IsVisible(const FVector& v, float radius) const
{
	int visible = 1;
	int complete = 1;

	const Plane* plane = frustum_planes;

	if (infinite) {
		complete = 0;

		for (int i = 0; visible && (i < NUM_FRUSTUM_PLANES); ++i, ++plane) {
			const float dot = FVector::DotProduct(v, plane->normal);
			visible = (dot >= CLIP_PLANE_EPSILON);
		}
	}
	else {
		for (int i = 0; visible && (i < NUM_FRUSTUM_PLANES); ++i, ++plane) {
			const float dot = FVector::DotProduct(v, plane->normal);

			visible = ((dot + radius) >= plane->distance);
			complete = complete && ((dot - radius) >= plane->distance);
		}
	}

	// Return values (legacy Starshatter behavior):
	// 0 = not visible
	// 1 = partially visible
	// 2 = completely inside
	return visible + complete;
}

// +--------------------------------------------------------------------+
// Clip the bounding point against the frustum and return non zero
// if at least partially inside.
// +--------------------------------------------------------------------+

int
SimProjector::IsBoxVisible(const FVector* p) const
{
	int outside = 0;

	// If all eight corners are outside of the same frustum plane, the box is not visible:
	const Plane* plane = frustum_planes;

	if (infinite) {
		for (int i = 0; (outside == 0) && (i < NUM_FRUSTUM_PLANES); ++i, ++plane) {

			int count = 0;
			for (int j = 0; j < 8; ++j) {
				const float dot = FVector::DotProduct(p[j], plane->normal);
				if (dot < CLIP_PLANE_EPSILON)
					++count;
			}

			// all 8 corners outside this plane:
			if (count == 8)
				outside = 1;
		}
	}
	else {
		for (int i = 0; (outside == 0) && (i < NUM_FRUSTUM_PLANES); ++i, ++plane) {

			int count = 0;
			for (int j = 0; j < 8; ++j) {
				const float dot = FVector::DotProduct(p[j], plane->normal);
				if (dot < plane->distance)
					++count;
			}

			// all 8 corners outside this plane:
			if (count == 8)
				outside = 1;
		}
	}

	// if not outside, then the box is visible:
	return outside ? 0 : 1;
}

// +--------------------------------------------------------------------+

float
SimProjector::ApparentRadius(const FVector& v, float radius) const
{
	FVector vloc = v;

	Transform(vloc); // transform in place
	return ProjectRadius(vloc, radius);
}

// +--------------------------------------------------------------------+
// Rotate a vector from viewspace to worldspace.
// +--------------------------------------------------------------------+

void
SimProjector::ViewToWorld(FVector& vin, FVector& vout)
{
	const FVector Vrt = vrt();
	const FVector Vup = vup();
	const FVector Vpn = vpn();

	vout.X = vin.X * Vrt.X + vin.Y * Vup.X + vin.Z * Vpn.X;
	vout.Y = vin.X * Vrt.Y + vin.Y * Vup.Y + vin.Z * Vpn.Y;
	vout.Z = vin.X * Vrt.Z + vin.Y * Vup.Z + vin.Z * Vpn.Z;
}
