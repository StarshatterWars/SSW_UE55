/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright (c) 2025-2026. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         CameraView.cpp
	AUTHOR:       Carlos Bott

	ORIGINAL AUTHOR: John DiCamillo
	ORIGINAL STUDIO: Destroyer Studios LLC

	OVERVIEW
	========
	3D Projection Camera View class
	uses abstract PolyRender class to draw the triangles
*/

#include "CameraView.h"

#include "Color.h"
#include "Window.h"
#include "SimScene.h"
#include "SimLight.h"
#include "Solid.h"
#include "Shadow.h"
#include "Sprite.h"
#include "Video.h"
#include "Screen.h"
#include "Game.h"

// Unreal render asset replacement:
#include "Engine/Texture2D.h"

// Unreal logging / math:
#include "Logging/LogMacros.h"
#include "Math/Vector.h"

DEFINE_LOG_CATEGORY_STATIC(LogCameraView, Log, All);

// +--------------------------------------------------------------------+

static Camera emergency_cam;
static SimScene  emergency_scene;

// +--------------------------------------------------------------------+

CameraView::CameraView(Window* c, Camera* cam, SimScene* s)
	: View(c)
	, camera(cam)
	, scene(s)
	, video(0)
	, camera_loc(FVector::ZeroVector)
	, cvrt(FVector::ZeroVector)
	, cvup(FVector::ZeroVector)
	, cvpn(FVector::ZeroVector)
	, projector(c, cam)
	, infinite(0)
	, width(0)
	, height(0)
	, projection_type(Video::PROJECTION_PERSPECTIVE)
	, lens_flare_enable(0)
	, lens_flare_dim(0)
	, halo_texture(0)
{
	elem_texture[0] = 0;
	elem_texture[1] = 0;
	elem_texture[2] = 0;

	if (!camera)
		camera = &emergency_cam;

	if (!scene)
		scene = &emergency_scene;

	if (window) {
		Rect r = window->GetRect();
		width = r.w;
		height = r.h;
	}
}

CameraView::~CameraView()
{
}

// +--------------------------------------------------------------------+

void
CameraView::UseCamera(Camera* cam)
{
	if (cam)
		camera = cam;
	else
		camera = &emergency_cam;

	projector.UseCamera(camera);
}

void
CameraView::UseScene(SimScene* s)
{
	if (s)
		scene = s;
	else
		scene = &emergency_scene;
}

void
CameraView::SetFieldOfView(double fov)
{
	projector.SetFieldOfView(fov);
}

double
CameraView::GetFieldOfView() const
{
	return projector.GetFieldOfView();
}

void
CameraView::SetProjectionType(DWORD pt)
{
	projector.SetOrthogonal(pt == Video::PROJECTION_ORTHOGONAL);
	projection_type = pt;
}

DWORD
CameraView::GetProjectionType() const
{
	return projection_type;
}

void
CameraView::OnWindowMove()
{
	if (!window)
		return;

	Rect r = window->GetRect();
	projector.UseWindow(window);

	width = r.w;
	height = r.h;
}

// +--------------------------------------------------------------------+
// Enable or disable lens flare effect, and provide textures for rendering
// +--------------------------------------------------------------------+

void
CameraView::LensFlare(int on, double dim)
{
	lens_flare_enable = on;
	lens_flare_dim = dim;
}

void
CameraView::LensFlareElements(UTexture2D* halo, UTexture2D* e1, UTexture2D* e2, UTexture2D* e3)
{
	if (halo)
		halo_texture = halo;

	if (e1)
		elem_texture[0] = e1;

	if (e2)
		elem_texture[1] = e2;

	if (e3)
		elem_texture[2] = e3;
}

// +--------------------------------------------------------------------+

int
CameraView::SetInfinite(int i)
{
	int old = infinite;
	infinite = i;
	projector.SetInfinite(i);
	return old;
}

// +--------------------------------------------------------------------+
// Compute the Depth of a Graphic
// +--------------------------------------------------------------------+

void
CameraView::FindDepth(Graphic* g)
{
	if (!g || !camera)
		return;

	if (infinite) {
		g->SetDepth(1.0e20f);
		return;
	}

	// Translate into a viewpoint-relative coordinate
	const FVector loc = (FVector)(g->Location() - camera->Pos());

	// Rotate into the view orientation
	const float z = (float)(loc * (FVector)camera->vpn());
	g->SetDepth(z);
}

// +--------------------------------------------------------------------+

void
CameraView::Refresh()
{
	// disabled:
	if (camera == &emergency_cam)
		return;

	// prologue:
	video = Video::GetInstance();
	if (!video || !window || !scene || !camera)
		return;

	cvrt = (FVector)camera->vrt();
	cvup = (FVector)camera->vup();
	cvpn = (FVector)camera->vpn();

	TranslateScene();
	MarkVisibleObjects();

	Rect old_rect;
	video->GetWindowRect(old_rect);

	video->SetCamera(camera);
	video->SetWindowRect(window->GetRect());
	video->SetProjection((float)GetFieldOfView(), 1.0f, 1.0e6f, projection_type);

	// project and render:
	RenderBackground();
	RenderScene();
	RenderForeground();
	RenderSprites();
	RenderLensFlare();

	UnTranslateScene();

	video->SetWindowRect(old_rect);
}

// +--------------------------------------------------------------------+
// Translate all objects and lights to camera relative coordinates:
// +--------------------------------------------------------------------+

void
CameraView::TranslateScene()
{
	camera_loc = (FVector)camera->Pos();

	ListIter<Graphic> g_iter = scene->Graphics();
	while (++g_iter) {
		Graphic* graphic = g_iter.value();

		if (graphic && !graphic->IsInfinite())
			graphic->TranslateBy((FVector)camera_loc);
	}

	g_iter.attach(scene->Foreground());
	while (++g_iter) {
		Graphic* graphic = g_iter.value();
		if (graphic)
			graphic->TranslateBy((FVector)camera_loc);
	}

	g_iter.attach(scene->Sprites());
	while (++g_iter) {
		Graphic* graphic = g_iter.value();

		if (graphic && !graphic->IsInfinite())
			graphic->TranslateBy((FVector)camera_loc);
	}

	ListIter<SimLight> l_iter = scene->Lights();
	while (++l_iter) {
		Light* light = l_iter.value();
		if (light)
			light->TranslateBy((FVector)camera_loc);
	}

	camera->MoveTo(0, 0, 0);
}

// +--------------------------------------------------------------------+
// Translate all objects and lights back to original positions:
// +--------------------------------------------------------------------+

void
CameraView::UnTranslateScene()
{
	const FVector reloc = -camera_loc;

	ListIter<Graphic> g_iter = scene->Graphics();
	while (++g_iter) {
		Graphic* graphic = g_iter.value();

		if (graphic && !graphic->IsInfinite())
			graphic->TranslateBy((FVector)reloc);
	}

	g_iter.attach(scene->Foreground());
	while (++g_iter) {
		Graphic* graphic = g_iter.value();
		if (graphic)
			graphic->TranslateBy((FVector)reloc);
	}

	g_iter.attach(scene->Sprites());
	while (++g_iter) {
		Graphic* graphic = g_iter.value();

		if (graphic && !graphic->IsInfinite())
			graphic->TranslateBy((FVector)reloc);
	}

	ListIter<SimLight> l_iter = scene->Lights();
	while (++l_iter) {
		SimLight* light = l_iter.value();
		if (light)
			light->TranslateBy((FVector)reloc);
	}

	camera->MoveTo((FVector)camera_loc);
}

// +--------------------------------------------------------------------+
// Mark visible objects
// +--------------------------------------------------------------------+

void
CameraView::MarkVisibleObjects()
{
	projector.StartFrame();
	graphics.clear();

	ListIter<Graphic> graphic_iter = scene->Graphics();
	while (++graphic_iter) {
		Graphic* graphic = graphic_iter.value();

		if (!graphic || graphic->Hidden())
			continue;

		if (graphic->CheckVisibility(projector)) {
			graphic->Update();
			graphics.append(graphic);
		}
		else {
			graphic->ProjectScreenRect(0);
		}
	}
}

void
CameraView::MarkVisibleLights(Graphic* graphic, DWORD flags)
{
	if (!graphic || !scene || !camera)
		return;

	if (flags < Graphic::RENDER_FIRST_LIGHT) {
		flags = flags | Graphic::RENDER_FIRST_LIGHT | Graphic::RENDER_ADD_LIGHT;
	}

	if (graphic->IsVisible()) {
		ListIter<SimLight> light_iter = scene->Lights();

		while (++light_iter) {
			SimLight* light = light_iter.value();
			if (!light)
				continue;

			bool bright_enough = light->Type() == SimLight::LIGHT_DIRECTIONAL ||
				light->Intensity() >= 1e9;

			if (!bright_enough) {
				const FVector test = (FVector)(graphic->Location() - light->Location());
				if (test.Size() < (float)(light->Intensity() * 10))
					bright_enough = true;
			}

			// turn off lights that won't be used this pass:
			if (light->CastsShadow()) {
				if ((flags & Graphic::RENDER_ADD_LIGHT) == 0)
					bright_enough = false;
			}
			else {
				if ((flags & Graphic::RENDER_FIRST_LIGHT) == 0)
					bright_enough = false;
			}

			double obs_radius = graphic->Radius();
			if (obs_radius < 100)
				obs_radius = 100;

			light->SetActive(bright_enough);
		}
	}
}

// +--------------------------------------------------------------------+

void
CameraView::RenderBackground()
{
	if (scene->Background().isEmpty())
		return;

	video->SetRenderState(Video::FILL_MODE, Video::FILL_SOLID);
	video->SetRenderState(Video::Z_ENABLE, FALSE);
	video->SetRenderState(Video::Z_WRITE_ENABLE, FALSE);
	video->SetRenderState(Video::STENCIL_ENABLE, FALSE);
	video->SetRenderState(Video::LIGHTING_ENABLE, TRUE);

	// solid items:
	ListIter<Graphic> iter = scene->Background();
	while (++iter) {
		Graphic* g = iter.value();

		if (g && !g->Hidden())
			Render(g, Graphic::RENDER_SOLID);
	}

	// blended items:
	iter.reset();
	while (++iter) {
		Graphic* g = iter.value();

		if (g && !g->Hidden())
			Render(g, Graphic::RENDER_ALPHA);
	}

	// glowing items:
	iter.reset();
	while (++iter) {
		Graphic* g = iter.value();

		if (g && !g->Hidden())
			Render(g, Graphic::RENDER_ADDITIVE);
	}
}

// +--------------------------------------------------------------------+

void
CameraView::RenderForeground()
{
	bool foregroundVisible = false;

	ListIter<Graphic> iter = scene->Foreground();
	while (++iter && !foregroundVisible) {
		Graphic* g = iter.value();
		if (g && !g->Hidden())
			foregroundVisible = true;
	}

	if (!foregroundVisible)
		return;

	video->SetRenderState(Video::FILL_MODE, Video::FILL_SOLID);
	video->SetRenderState(Video::Z_ENABLE, TRUE);
	video->SetRenderState(Video::Z_WRITE_ENABLE, TRUE);
	video->SetRenderState(Video::STENCIL_ENABLE, FALSE);
	video->SetRenderState(Video::LIGHTING_ENABLE, TRUE);
	video->SetProjection((float)GetFieldOfView(), 1.0f, 1.0e6f, projection_type);

	if (video->IsShadowEnabled() || video->IsBumpMapEnabled()) {
		// solid items, ambient and non-shadow lights:
		iter.reset();
		while (++iter) {
			Graphic* g = iter.value();
			Render(g, Graphic::RENDER_SOLID | Graphic::RENDER_FIRST_LIGHT);
		}

		video->SetAmbient(Color::Black);
		video->SetRenderState(Video::LIGHTING_PASS, 2);

		// solid items, shadow lights:
		iter.reset();
		while (++iter) {
			Graphic* g = iter.value();
			Render(g, Graphic::RENDER_SOLID | Graphic::RENDER_ADD_LIGHT);
		}
	}
	else {
		// solid items:
		iter.reset();
		while (++iter) {
			Graphic* g = iter.value();
			Render(g, Graphic::RENDER_SOLID);
		}
	}

	video->SetAmbient(scene->Ambient());
	video->SetRenderState(Video::LIGHTING_PASS, 0);
	video->SetRenderState(Video::STENCIL_ENABLE, FALSE);
	video->SetRenderState(Video::Z_ENABLE, TRUE);
	video->SetRenderState(Video::Z_WRITE_ENABLE, FALSE);

	// blended items:
	iter.reset();
	while (++iter) {
		Graphic* g = iter.value();
		Render(g, Graphic::RENDER_ALPHA);
		if (g) g->ProjectScreenRect(&projector);
	}

	// glowing items:
	iter.reset();
	while (++iter) {
		Graphic* g = iter.value();
		Render(g, Graphic::RENDER_ADDITIVE);
		if (g) g->ProjectScreenRect(&projector);
	}
}

// +--------------------------------------------------------------------+

void
CameraView::RenderSprites()
{
	if (scene->Sprites().isEmpty())
		return;

	video->SetRenderState(Video::FILL_MODE, Video::FILL_SOLID);
	video->SetRenderState(Video::Z_ENABLE, TRUE);
	video->SetRenderState(Video::Z_WRITE_ENABLE, FALSE);
	video->SetRenderState(Video::STENCIL_ENABLE, FALSE);
	video->SetRenderState(Video::LIGHTING_ENABLE, TRUE);

	// compute depth:
	ListIter<Graphic> iter = scene->Sprites();
	while (++iter) {
		Graphic* g = iter.value();
		if (g && g->IsVisible() && !g->Hidden()) {
			FindDepth(g);
		}
	}

	// sort the list:
	scene->Sprites().sort();

	// blended items:
	iter.reset();
	while (++iter) {
		Graphic* g = iter.value();
		Render(g, Graphic::RENDER_ALPHA);
	}

	// glowing items:
	iter.reset();
	while (++iter) {
		Graphic* g = iter.value();
		Render(g, Graphic::RENDER_ADDITIVE);
	}
}

// +--------------------------------------------------------------------+
// Render the whole scene, sorted back to front
// +--------------------------------------------------------------------+

void
CameraView::RenderScene()
{
	if (graphics.isEmpty())
		return;

	// compute depth:
	ListIter<Graphic> iter = graphics;
	while (++iter) {
		Graphic* g = iter.value();
		if (g && !g->Hidden()) {
			FindDepth(g);

			if (g->IsSolid()) {
				Solid* solid = (Solid*)g;

				solid->SelectDetail(&projector);

				if (video->IsShadowEnabled()) {
					MarkVisibleLights(solid, Graphic::RENDER_ADD_LIGHT);
					solid->UpdateShadows(scene->Lights());
				}
			}
		}
	}

	// sort the list:
	graphics.sort();

	Graphic* g = graphics.last();
	if (g && g->Depth() > 5e6) {
		RenderSceneObjects(true);
		video->ClearDepthBuffer();
	}

	RenderSceneObjects(false);
}

void
CameraView::RenderSceneObjects(bool distant)
{
	ListIter<Graphic> iter = graphics;

	video->SetAmbient(scene->Ambient());
	video->SetRenderState(Video::FILL_MODE, Video::FILL_SOLID);
	video->SetRenderState(Video::Z_ENABLE, TRUE);
	video->SetRenderState(Video::Z_WRITE_ENABLE, TRUE);
	video->SetRenderState(Video::LIGHTING_ENABLE, TRUE);

	if (distant)
		video->SetProjection((float)GetFieldOfView(), 5.0e6f, 1.0e12f, projection_type);
	else
		video->SetProjection((float)GetFieldOfView(), 1.0f, 1.0e6f, projection_type);

	if (video->IsShadowEnabled() || video->IsBumpMapEnabled()) {
		// solid items, ambient and non-shadow lights:
		iter.reset();
		while (++iter) {
			Graphic* g = iter.value();

			if ((distant && g->Depth() > 5e6) || (!distant && g->Depth() < 5e6)) {
				Render(g, Graphic::RENDER_SOLID | Graphic::RENDER_FIRST_LIGHT);
			}
		}

		// send shadows to stencil buffer:
		if (video->IsShadowEnabled()) {
			iter.reset();
			while (++iter) {
				Graphic* g = iter.value();
				if ((distant && g->Depth() > 5e6) || (!distant && g->Depth() < 5e6)) {
					if (g->IsSolid()) {
						Solid* solid = (Solid*)g;

						ListIter<Shadow> shadow_iter = solid->GetShadows();
						while (++shadow_iter) {
							Shadow* shadow = shadow_iter.value();
							if (shadow)
								shadow->Render(video);
						}
					}
				}
			}
		}

		video->SetAmbient(Color::Black);
		video->SetRenderState(Video::LIGHTING_PASS, 2);
		video->SetRenderState(Video::STENCIL_ENABLE, TRUE);

		// solid items, shadow lights:
		iter.reset();
		while (++iter) {
			Graphic* g = iter.value();

			if ((distant && g->Depth() > 5e6) || (!distant && g->Depth() < 5e6)) {
				Render(g, Graphic::RENDER_SOLID | Graphic::RENDER_ADD_LIGHT);
			}
		}
	}
	else {
		// solid items:
		iter.reset();
		while (++iter) {
			Graphic* g = iter.value();

			if ((distant && g->Depth() > 5e6) || (!distant && g->Depth() < 5e6)) {
				Render(g, Graphic::RENDER_SOLID);
			}
		}
	}

	video->SetAmbient(scene->Ambient());
	video->SetRenderState(Video::LIGHTING_PASS, 0);
	video->SetRenderState(Video::STENCIL_ENABLE, FALSE);
	video->SetRenderState(Video::Z_ENABLE, TRUE);
	video->SetRenderState(Video::Z_WRITE_ENABLE, FALSE);

	// blended items:
	iter.reset();
	while (++iter) {
		Graphic* g = iter.value();

		if ((distant && g->Depth() > 5e6) || (!distant && g->Depth() < 5e6)) {
			Render(g, Graphic::RENDER_ALPHA);
			g->ProjectScreenRect(&projector);
		}
	}

	// glowing items:
	iter.reset();
	while (++iter) {
		Graphic* g = iter.value();

		if ((distant && g->Depth() > 5e6) || (!distant && g->Depth() < 5e6)) {
			Render(g, Graphic::RENDER_ADDITIVE);
			g->ProjectScreenRect(&projector);
		}
	}
}

void
CameraView::Render(Graphic* g, DWORD flags)
{
	if (g && g->IsVisible() && !g->Hidden()) {
		if (g->IsSolid()) {
			MarkVisibleLights(g, flags);
			video->SetLights(scene->Lights());
		}

		g->Render(video, flags);
	}
}

// +--------------------------------------------------------------------+
// Draw the lens flare effect, if enabled and light source visible
// +--------------------------------------------------------------------+

void
CameraView::RenderLensFlare()
{
	if (!lens_flare_enable || lens_flare_dim < 0.01)
		return;

	if (!halo_texture)
		return;

	video->SetRenderState(Video::STENCIL_ENABLE, FALSE);
	video->SetRenderState(Video::Z_ENABLE, FALSE);
	video->SetRenderState(Video::Z_WRITE_ENABLE, FALSE);

	const FVector center((float)width / 2.0f, (float)height / 2.0f, 1.0f);

	ListIter<SimLight> light_iter = scene->Lights();
	while (++light_iter) {
		SimLight* light = light_iter.value();

		if (!light || !light->IsActive())
			continue;

		if (light->Type() == Light::LIGHT_DIRECTIONAL && light->Intensity() < 1)
			continue;

		const double distance = (light->Location() - camera->Pos()).Length();

		// only do lens flare for the sun:
		if (distance > 1e9) {
			if (projector.IsVisible(light->Location(), 1.0f)) {
				FVector sun_pos = light->Location();

				if (light->CastsShadow() && scene->IsLightObscured(camera->Pos(), sun_pos, -1))
					continue;

				projector.Transform(sun_pos);

				if (sun_pos.z < 100)
					continue;

				projector.Project(sun_pos, false);

				int x = (int)(sun_pos.x);
				int y = (int)(sun_pos.y);
				int w = (int)(window->Width() / 4.0);
				int h = w;

				// HALO:
				window->DrawTexture(x - w, y - h, x + w, y + h, halo_texture, Video::BLEND_ADDITIVE);

				// lens elements:
				if (elem_texture[0]) {
					const FVector sun((float)sun_pos.x, (float)sun_pos.y, (float)sun_pos.z);

					FVector vector = center - sun;
					const float vlen = vector.Size();
					vector.Normalize();

					static int   nelem = 12;
					static int   elem_indx[] = { 0, 1, 1, 1, 0, 0, 0, 0, 2, 0, 0, 2 };
					static float elem_dist[] = { -0.2f, 0.5f, 0.55f, 0.62f, 1.23f, 1.33f, 1.35f, 0.8f, 0.9f, 1.4f, 1.7f, 1.8f };
					static float elem_size[] = { 0.3f, 0.2f, 0.4f,  0.3f,  0.4f,  0.2f,  0.6f,  0.1f, 0.1f, 1.6f, 1.0f, 0.2f };

					for (int elem = 0; elem < nelem; elem++) {
						UTexture2D* tex = elem_texture[elem_indx[elem]];
						if (!tex)
							tex = elem_texture[0];

						const FVector flare_pos = sun + (vector * elem_dist[elem] * vlen);

						x = (int)flare_pos.X;
						y = (int)flare_pos.Y;
						w = (int)(window->Width() / 8.0 * elem_size[elem]);
						h = w;

						window->DrawTexture(x - w, y - h, x + w, y + h, tex, Video::BLEND_ADDITIVE);
					}
				}
			}
		}
	}
}

// +--------------------------------------------------------------------+
// Rotate and translate a plane in world space to view space.
// +--------------------------------------------------------------------+

void
CameraView::WorldPlaneToView(Plane& plane)
{
	// Determine the distance from the viewpoint
	FVector tnormal = plane.normal;

	if (!infinite)
		plane.distance -= (float)(camera->Pos() * tnormal);

	// Rotate the normal into view orientation
	plane.normal.x = tnormal * (FVector)cvrt;
	plane.normal.y = tnormal * (FVector)cvup;
	plane.normal.z = tnormal * (FVector)cvpn;
}

void
CameraView::SetDepthScale(float scale)
{
	projector.SetDepthScale(scale);
}
