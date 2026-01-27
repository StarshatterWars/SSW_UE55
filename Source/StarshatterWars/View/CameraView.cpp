/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

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

static Camera   emergency_cam;
static SimScene emergency_scene;

// +--------------------------------------------------------------------+

UCameraView::UCameraView(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , projector(nullptr, nullptr) // will be re-bound in InitializeView()
{
}

void UCameraView::InitializeView(Window* InWindow, Camera* InCamera, SimScene* InScene)
{
    window = InWindow;
    camera = InCamera ? InCamera : &emergency_cam;
    scene = InScene ? InScene : &emergency_scene;

    // Rebind projector now that we have a window + camera:
    projector = SimProjector(window, camera);

    lens_flare_enable = 0;
    halo_bitmap = nullptr;

    elem_bitmap[0] = nullptr;
    elem_bitmap[1] = nullptr;
    elem_bitmap[2] = nullptr;

    if (window)
    {
        Rect r = window->GetRect();
        width = r.w;
        height = r.h;
        projector.UseWindow(window);
    }
}

void UCameraView::UseCamera(Camera* cam)
{
    camera = cam ? cam : &emergency_cam;
    projector.UseCamera(camera);
}

void UCameraView::UseScene(SimScene* s)
{
    scene = s ? s : &emergency_scene;
}

void UCameraView::SetFieldOfView(double fov)
{
    projector.SetFieldOfView(fov);
}

double UCameraView::GetFieldOfView() const
{
    return projector.GetFieldOfView();
}

void UCameraView::SetProjectionType(DWORD pt)
{
    projector.SetOrthogonal(pt == Video::PROJECTION_ORTHOGONAL);
    projection_type = pt;
}

DWORD UCameraView::GetProjectionType() const
{
    return projection_type;
}

void UCameraView::OnWindowMove()
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

void UCameraView::LensFlare(int on, double dim)
{
    lens_flare_enable = on;
    lens_flare_dim = dim;
}

void UCameraView::LensFlareElements(Bitmap* halo, Bitmap* e1, Bitmap* e2, Bitmap* e3)
{
    if (halo) halo_bitmap = halo;
    if (e1)   elem_bitmap[0] = e1;
    if (e2)   elem_bitmap[1] = e2;
    if (e3)   elem_bitmap[2] = e3;
}

// +--------------------------------------------------------------------+
// Compute the Depth of a Graphic
// +--------------------------------------------------------------------+

void UCameraView::FindDepth(Graphic* g)
{
    if (!g || !camera)
        return;

    if (infinite)
    {
        g->SetDepth(1.0e20f);
        return;
    }

    // Viewpoint-relative vector (world space):
    const FVector loc = g->Location() - camera->Pos();

    // Starshatter used (Vec3 * Vec3) as dot product.
    const FVector vpnv = camera->vpn();
    const float z = FVector::DotProduct(loc, vpnv);

    g->SetDepth(z);
}

// +--------------------------------------------------------------------+

void UCameraView::Refresh()
{
    // Disabled:
    if (camera == &emergency_cam)
        return;

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

void UCameraView::TranslateScene()
{
    camera_loc = (FVector)camera->Pos();

    ListIter<Graphic> g_iter = scene->Graphics();
    while (++g_iter)
    {
        Graphic* graphic = g_iter.value();
        if (graphic && !graphic->IsInfinite())
            graphic->TranslateBy(camera_loc);
    }

    g_iter.attach(scene->Foreground());
    while (++g_iter)
    {
        Graphic* graphic = g_iter.value();
        if (graphic)
            graphic->TranslateBy(camera_loc);
    }

    g_iter.attach(scene->Sprites());
    while (++g_iter)
    {
        Graphic* graphic = g_iter.value();
        if (graphic && !graphic->IsInfinite())
            graphic->TranslateBy(camera_loc);
    }

    ListIter<SimLight> l_iter = scene->Lights();
    while (++l_iter)
    {
        SimLight* light = l_iter.value();
        if (light)
            light->TranslateBy(camera_loc);
    }

    camera->MoveTo(0, 0, 0);
}

// +--------------------------------------------------------------------+
// Translate all objects and lights back to original positions:
// +--------------------------------------------------------------------+

void UCameraView::UnTranslateScene()
{
    const FVector reloc = -camera_loc;

    ListIter<Graphic> g_iter = scene->Graphics();
    while (++g_iter)
    {
        Graphic* graphic = g_iter.value();
        if (graphic && !graphic->IsInfinite())
            graphic->TranslateBy(reloc);
    }

    g_iter.attach(scene->Foreground());
    while (++g_iter)
    {
        Graphic* graphic = g_iter.value();
        if (graphic)
            graphic->TranslateBy(reloc);
    }

    g_iter.attach(scene->Sprites());
    while (++g_iter)
    {
        Graphic* graphic = g_iter.value();
        if (graphic && !graphic->IsInfinite())
            graphic->TranslateBy(reloc);
    }

    ListIter<SimLight> l_iter = scene->Lights();
    while (++l_iter)
    {
        SimLight* light = l_iter.value();
        if (light)
            light->TranslateBy(reloc);
    }

    camera->MoveTo(camera_loc);
}

// +--------------------------------------------------------------------+
// Mark visible objects
// +--------------------------------------------------------------------+

void UCameraView::MarkVisibleObjects()
{
    projector.StartFrame();
    graphics.clear();

    ListIter<Graphic> graphic_iter = scene->Graphics();
    while (++graphic_iter)
    {
        Graphic* graphic = graphic_iter.value();

        if (!graphic || graphic->Hidden())
            continue;

        if (graphic->CheckVisibility(projector))
        {
            graphic->Update();
            graphics.append(graphic);
        }
        else
        {
            graphic->ProjectScreenRect(0);
        }
    }
}

void UCameraView::MarkVisibleLights(Graphic* graphic, DWORD flags)
{
    if (!graphic || !scene || !camera)
        return;

    if (flags < Graphic::RENDER_FIRST_LIGHT)
        flags = flags | Graphic::RENDER_FIRST_LIGHT | Graphic::RENDER_ADD_LIGHT;

    if (graphic->IsVisible())
    {
        ListIter<SimLight> light_iter = scene->Lights();
        while (++light_iter)
        {
            SimLight* light = light_iter.value();
            if (!light)
                continue;

            bool bright_enough =
                light->Type() == SimLight::LIGHT_DIRECTIONAL ||
                light->Intensity() >= 1e9;

            if (!bright_enough)
            {
                const FVector test = (graphic->Location() - light->Location());
                if (test.Size() < (float)(light->Intensity() * 10))
                    bright_enough = true;
            }

            if (light->CastsShadow())
            {
                if ((flags & Graphic::RENDER_ADD_LIGHT) == 0)
                    bright_enough = false;
            }
            else
            {
                if ((flags & Graphic::RENDER_FIRST_LIGHT) == 0)
                    bright_enough = false;
            }

            double obs_radius = graphic->Radius();
            if (obs_radius < 100) obs_radius = 100;

            light->SetActive(bright_enough);
        }
    }
}

// --------------------------------------------------------------------
// Rendering methods (unchanged logic)
// --------------------------------------------------------------------

void UCameraView::RenderBackground()
{
    if (scene->Background().isEmpty())
        return;

    video->SetRenderState(Video::FILL_MODE, Video::FILL_SOLID);
    video->SetRenderState(Video::Z_ENABLE, FALSE);
    video->SetRenderState(Video::Z_WRITE_ENABLE, FALSE);
    video->SetRenderState(Video::STENCIL_ENABLE, FALSE);
    video->SetRenderState(Video::LIGHTING_ENABLE, TRUE);

    ListIter<Graphic> iter = scene->Background();
    while (++iter)
    {
        Graphic* g = iter.value();
        if (g && !g->Hidden())
            Render(g, Graphic::RENDER_SOLID);
    }

    iter.reset();
    while (++iter)
    {
        Graphic* g = iter.value();
        if (g && !g->Hidden())
            Render(g, Graphic::RENDER_ALPHA);
    }

    iter.reset();
    while (++iter)
    {
        Graphic* g = iter.value();
        if (g && !g->Hidden())
            Render(g, Graphic::RENDER_ADDITIVE);
    }
}

void UCameraView::RenderForeground()
{
    bool foregroundVisible = false;

    ListIter<Graphic> iter = scene->Foreground();
    while (++iter && !foregroundVisible)
    {
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

    if (video->IsShadowEnabled() || video->IsBumpMapEnabled())
    {
        iter.reset();
        while (++iter)
        {
            Graphic* g = iter.value();
            Render(g, Graphic::RENDER_SOLID | Graphic::RENDER_FIRST_LIGHT);
        }

        video->SetAmbient(FColor::Black);
        video->SetRenderState(Video::LIGHTING_PASS, 2);

        iter.reset();
        while (++iter)
        {
            Graphic* g = iter.value();
            Render(g, Graphic::RENDER_SOLID | Graphic::RENDER_ADD_LIGHT);
        }
    }
    else
    {
        iter.reset();
        while (++iter)
        {
            Graphic* g = iter.value();
            Render(g, Graphic::RENDER_SOLID);
        }
    }

    video->SetAmbient(scene->Ambient());
    video->SetRenderState(Video::LIGHTING_PASS, 0);
    video->SetRenderState(Video::STENCIL_ENABLE, FALSE);
    video->SetRenderState(Video::Z_ENABLE, TRUE);
    video->SetRenderState(Video::Z_WRITE_ENABLE, FALSE);

    iter.reset();
    while (++iter)
    {
        Graphic* g = iter.value();
        Render(g, Graphic::RENDER_ALPHA);
        if (g) g->ProjectScreenRect(&projector);
    }

    iter.reset();
    while (++iter)
    {
        Graphic* g = iter.value();
        Render(g, Graphic::RENDER_ADDITIVE);
        if (g) g->ProjectScreenRect(&projector);
    }
}

void UCameraView::RenderSprites()
{
    if (scene->Sprites().isEmpty())
        return;

    video->SetRenderState(Video::FILL_MODE, Video::FILL_SOLID);
    video->SetRenderState(Video::Z_ENABLE, TRUE);
    video->SetRenderState(Video::Z_WRITE_ENABLE, FALSE);
    video->SetRenderState(Video::STENCIL_ENABLE, FALSE);
    video->SetRenderState(Video::LIGHTING_ENABLE, TRUE);

    ListIter<Graphic> iter = scene->Sprites();
    while (++iter)
    {
        Graphic* g = iter.value();
        if (g && g->IsVisible() && !g->Hidden())
            FindDepth(g);
    }

    scene->Sprites().sort();

    iter.reset();
    while (++iter)
        Render(iter.value(), Graphic::RENDER_ALPHA);

    iter.reset();
    while (++iter)
        Render(iter.value(), Graphic::RENDER_ADDITIVE);
}

void UCameraView::RenderScene()
{
    if (graphics.isEmpty())
        return;

    ListIter<Graphic> iter = graphics;
    while (++iter)
    {
        Graphic* g = iter.value();
        if (g && !g->Hidden())
        {
            FindDepth(g);

            if (g->IsSolid())
            {
                Solid* solid = (Solid*)g;
                solid->SelectDetail(&projector);

                if (video->IsShadowEnabled())
                {
                    MarkVisibleLights(solid, Graphic::RENDER_ADD_LIGHT);
                    solid->UpdateShadows(scene->Lights());
                }
            }
        }
    }

    graphics.sort();

    Graphic* g = graphics.last();
    if (g && g->Depth() > 5e6)
    {
        RenderSceneObjects(true);
        video->ClearDepthBuffer();
    }

    RenderSceneObjects(false);
}

void UCameraView::RenderSceneObjects(bool distant)
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

    if (video->IsShadowEnabled() || video->IsBumpMapEnabled())
    {
        iter.reset();
        while (++iter)
        {
            Graphic* g = iter.value();
            if (!g) continue;

            if ((distant && g->Depth() > 5e6) || (!distant && g->Depth() < 5e6))
                Render(g, Graphic::RENDER_SOLID | Graphic::RENDER_FIRST_LIGHT);
        }

        if (video->IsShadowEnabled())
        {
            iter.reset();
            while (++iter)
            {
                Graphic* g = iter.value();
                if (!g) continue;

                if ((distant && g->Depth() > 5e6) || (!distant && g->Depth() < 5e6))
                {
                    if (g->IsSolid())
                    {
                        Solid* solid = (Solid*)g;

                        ListIter<Shadow> shadow_iter = solid->GetShadows();
                        while (++shadow_iter)
                        {
                            Shadow* shadow = shadow_iter.value();
                            if (shadow) shadow->Render(video);
                        }
                    }
                }
            }
        }

        video->SetAmbient(FColor::Black);
        video->SetRenderState(Video::LIGHTING_PASS, 2);
        video->SetRenderState(Video::STENCIL_ENABLE, TRUE);

        iter.reset();
        while (++iter)
        {
            Graphic* g = iter.value();
            if (!g) continue;

            if ((distant && g->Depth() > 5e6) || (!distant && g->Depth() < 5e6))
                Render(g, Graphic::RENDER_SOLID | Graphic::RENDER_ADD_LIGHT);
        }
    }
    else
    {
        iter.reset();
        while (++iter)
        {
            Graphic* g = iter.value();
            if (!g) continue;

            if ((distant && g->Depth() > 5e6) || (!distant && g->Depth() < 5e6))
                Render(g, Graphic::RENDER_SOLID);
        }
    }

    video->SetAmbient(scene->Ambient());
    video->SetRenderState(Video::LIGHTING_PASS, 0);
    video->SetRenderState(Video::STENCIL_ENABLE, FALSE);
    video->SetRenderState(Video::Z_ENABLE, TRUE);
    video->SetRenderState(Video::Z_WRITE_ENABLE, FALSE);

    iter.reset();
    while (++iter)
    {
        Graphic* g = iter.value();
        if (!g) continue;

        if ((distant && g->Depth() > 5e6) || (!distant && g->Depth() < 5e6))
        {
            Render(g, Graphic::RENDER_ALPHA);
            g->ProjectScreenRect(&projector);
        }
    }

    iter.reset();
    while (++iter)
    {
        Graphic* g = iter.value();
        if (!g) continue;

        if ((distant && g->Depth() > 5e6) || (!distant && g->Depth() < 5e6))
        {
            Render(g, Graphic::RENDER_ADDITIVE);
            g->ProjectScreenRect(&projector);
        }
    }
}

void UCameraView::Render(Graphic* g, DWORD flags)
{
    if (g && g->IsVisible() && !g->Hidden())
    {
        if (g->IsSolid())
        {
            MarkVisibleLights(g, flags);
            video->SetLights(scene->Lights());
        }

        g->Render(video, flags);
    }
}

// NOTE: RenderLensFlare() and WorldPlaneToView() can remain exactly as you pasted.
// Keep them unchanged; they compile once the class is UCameraView and projector/window/camera are valid.

void UCameraView::WorldPlaneToView(Plane& plane)
{
    const FVector WorldNormal = plane.normal;

    if (!infinite && camera)
        plane.distance -= FVector::DotProduct(camera->Pos(), WorldNormal);

    plane.normal.X = FVector::DotProduct(WorldNormal, cvrt);
    plane.normal.Y = FVector::DotProduct(WorldNormal, cvup);
    plane.normal.Z = FVector::DotProduct(WorldNormal, cvpn);
}

void UCameraView::SetDepthScale(float scale)
{
    projector.SetDepthScale(scale);
}

int UCameraView::SetInfinite(int i)
{
    infinite = i;
    return infinite;
}
