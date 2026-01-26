/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         TacRefDlg.cpp
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    Tactical Reference screen (UMG UserWidget) — Unreal port of the legacy
    TacRefDlg tactical reference dialog.
*/


#include "TacRefDlg.h"
#include "GameStructs.h"

// UMG:
#include "Components/Button.h"
#include "Components/ListView.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"

#include "Engine/Texture2D.h"

#include "Game.h"

// Legacy / Starshatter core:
#include "MenuScreen.h"
#include "Campaign.h"
#include "Mission.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "WeaponDesign.h"
#include "WeaponGroup.h"

#include "ParseUtil.h"
#include "FormatUtil.h"

DEFINE_LOG_CATEGORY_STATIC(LogTacRefDlg, Log, All);

// +--------------------------------------------------------------------+
// Note on rendering / preview:
//
// The original dialog used Scene/CameraView/Solid/Light to render a 3D preview
// into an ActiveWindow ("beauty").
// In Unreal, the usual equivalent is a SceneCaptureComponent2D rendering into
// a UTextureRenderTarget2D, displayed by the UImage "Beauty".
//
// This port preserves the *logic* and UI flow, but it does not recreate the
// legacy nGenEx scene graph in this file. Hook your preview pipeline where
// marked (e.g., UpdateCamera/SelectShip/SelectWeapon).
// +--------------------------------------------------------------------+

// Local helper struct preserved from legacy:
struct WepGroup
{
    Text name;
    int  count;

    WepGroup() : count(0) {}
};

static WepGroup* FindWepGroup(WepGroup* weapons, const char* name)
{
    if (!weapons || !name || !*name)
        return nullptr;

    for (int i = 0; i < 8; ++i) {
        WepGroup* g = &weapons[i];

        // First time: claim the slot:
        if (g->name.length() == 0) {
            g->name = name;
            return g;
        }

        // Match:
        if (g->name == name)
            return g;
    }

    return nullptr;
}

// +--------------------------------------------------------------------+

UTacRefDlg::UTacRefDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    mode = MODE_NONE;
    radius = 100.0;
    cam_zoom = 2.5;
    cam_az = -PI / 6.0;
    cam_el = PI / 7.0;
    mouse_x = 0;
    mouse_y = 0;
    update_scene = false;
    captured = false;
    ship_index = 0;
    weap_index = 0;
}

// +--------------------------------------------------------------------+

void UTacRefDlg::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (BtnClose) BtnClose->OnClicked.AddDynamic(this, &UTacRefDlg::HandleCloseClicked);
    if (BtnShips) BtnShips->OnClicked.AddDynamic(this, &UTacRefDlg::HandleShipsClicked);
    if (BtnWeaps) BtnWeaps->OnClicked.AddDynamic(this, &UTacRefDlg::HandleWeaponsClicked);

    // List selection handling is project-specific with UListView (items are UObject-based).
    // If you want pure native + Starshatter pointers, keep a parallel native list and drive
    // selection manually (e.g., from a wrapper UObject that stores an index).
}

void UTacRefDlg::NativeConstruct()
{
    Super::NativeConstruct();
}

void UTacRefDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    ExecFrame();
}

// +--------------------------------------------------------------------+

void UTacRefDlg::Show()
{
    // Legacy behavior: update_scene = !shown; there is no "shown" flag here.
    // Treat first-show as update request:
    update_scene = true;

    if (update_scene) {
        // Default mode to ships:
        mode = MODE_SHIPS;
        HandleShipsClicked();
    }
}

// +--------------------------------------------------------------------+

void UTacRefDlg::SelectShip(const ShipDesign* design)
{
    // ---- Preview rendering hook --------------------------------------
    // Legacy cleared scene graphics and rebuilt Solids per LOD.
    // In Unreal, swap the preview mesh / render target content here.
    //
    // Example integration points (your project-specific pipeline):
    // - Set a preview actor mesh based on ShipDesign
    // - Update a SceneCaptureComponent2D transform using UpdateCamera()
    // - Display RT in Beauty (UImage)

    if (design) {
        radius = design->radius;
        UpdateCamera();
    }

    if (TxtCaption) {
        TxtCaption->SetText(FText::GetEmpty());

        if (design) {
            const FString Caption = FString::Printf(TEXT("%s %s"),
                ANSI_TO_TCHAR(design->abrv),
                ANSI_TO_TCHAR(design->DisplayName()));
            TxtCaption->SetText(FText::FromString(Caption));
        }
    }

    if (TxtStats) {
        TxtStats->SetText(FText::GetEmpty());

        if (design) {
            Text desc;
            char txt[256];

            sprintf_s(txt, "%s\t\t\t%s\n",
                Game::GetText("tacref.type").data(),
                Ship::ClassName(design->type));
            desc += txt;

            sprintf_s(txt, "%s\t\t\t%s\n",
                Game::GetText("tacref.class").data(),
                design->DisplayName());
            desc += txt;

            desc += Game::GetText("tacref.length");
            desc += "\t\t";

            if (design->type < (int)CLASSIFICATION::STATION)
                FormatNumber(txt, design->radius / 2);
            else
                FormatNumber(txt, design->radius * 2);

            strcat_s(txt, " m\n");
            desc += txt;

            desc += Game::GetText("tacref.mass");
            desc += "\t\t\t";

            FormatNumber(txt, design->mass);
            strcat_s(txt, " T\n");
            desc += txt;

            desc += Game::GetText("tacref.hull");
            desc += "\t\t\t";

            FormatNumber(txt, design->integrity);
            strcat_s(txt, "\n");
            desc += txt;

            if (design->weapons.size()) {
                desc += Game::GetText("tacref.weapons");

                WepGroup groups[8];
                for (int w = 0; w < design->weapons.size(); ++w) {
                    Weapon* gun = design->weapons[w];
                    WepGroup* group = FindWepGroup(groups, gun->Group());

                    if (group)
                        group->count++;
                }

                for (int g = 0; g < 8; ++g) {
                    WepGroup* group = &groups[g];
                    if (group && group->count) {
                        sprintf_s(txt, "\t\t%s (%d)\n\t\t", group->name.data(), group->count);
                        desc += txt;

                        for (int w = 0; w < design->weapons.size(); ++w) {
                            Weapon* gun = design->weapons[w];

                            if (group->name == gun->Group()) {
                                sprintf_s(txt, "\t\t\t%s\n\t\t", (const char*)gun->Design()->name);
                                desc += txt;
                            }
                        }
                    }
                }

                desc += "\n";
            }

            TxtStats->SetText(FText::FromString(FString(desc.data())));
        }
    }

    if (TxtDescription) {
        if (design && design->description.length()) {
            TxtDescription->SetText(FText::FromString(FString(design->description.data())));
        }
        else {
            TxtDescription->SetText(FText::FromString(FString(Game::GetText("tacref.mass").data())));
        }
    }
}

// +--------------------------------------------------------------------+

void UTacRefDlg::SelectWeapon(const WeaponDesign* design)
{
    // ---- Image preview hook ------------------------------------------
    // Legacy set an ImgView picture (bitmap).
    // In Unreal, map design->beauty_img to a UTexture2D (or a render asset)
    // and set it on Beauty->SetBrushFromTexture(Texture).

    if (TxtCaption) {
        TxtCaption->SetText(FText::GetEmpty());
        if (design)
            TxtCaption->SetText(FText::FromString(FString(design->name)));
    }

    if (TxtStats) {
        TxtStats->SetText(FText::GetEmpty());

        if (design) {
            Text desc;
            char txt[256];

            desc = Game::GetText("tacref.name");
            desc += "\t";
            desc += design->name;
            desc += "\n";

            desc += Game::GetText("tacref.type");
            desc += "\t\t";

            if (design->damage < 1)
                desc += Game::GetText("tacref.wep.other");
            else if (design->beam)
                desc += Game::GetText("tacref.wep.beam");
            else if (design->primary)
                desc += Game::GetText("tacref.wep.bolt");
            else if (design->drone)
                desc += Game::GetText("tacref.wep.drone");
            else if (design->guided)
                desc += Game::GetText("tacref.wep.guided");
            else
                desc += Game::GetText("tacref.wep.missile");

            if (design->turret_model && design->damage >= 1) {
                desc += " ";
                desc += Game::GetText("tacref.wep.turret");
                desc += "\n";
            }
            else {
                desc += "\n";
            }

            desc += Game::GetText("tacref.targets");
            desc += "\t";

            if ((design->target_type & (int)CLASSIFICATION::DROPSHIPS) != 0) {
                if ((design->target_type & (int)CLASSIFICATION::STARSHIPS) != 0) {
                    if ((design->target_type & (int)CLASSIFICATION::GROUND_UNITS) != 0) {
                        desc += Game::GetText("tacref.targets.fsg");
                    }
                    else {
                        desc += Game::GetText("tacref.targets.fs");
                    }
                }
                else {
                    if ((design->target_type & (int)CLASSIFICATION::GROUND_UNITS) != 0) {
                        desc += Game::GetText("tacref.targets.fg");
                    }
                    else {
                        desc += Game::GetText("tacref.targets.f");
                    }
                }
            }
            else if ((design->target_type & (int)CLASSIFICATION::STARSHIPS) != 0) {
                if ((design->target_type & (int)CLASSIFICATION::GROUND_UNITS) != 0) {
                    desc += Game::GetText("tacref.targets.sg");
                }
                else {
                    desc += Game::GetText("tacref.targets.s");
                }
            }
            else if ((design->target_type & (int)CLASSIFICATION::GROUND_UNITS) != 0) {
                desc += Game::GetText("tacref.targets.g");
            }

            desc += "\n";

            desc += Game::GetText("tacref.speed");
            desc += "\t";

            FormatNumber(txt, design->speed);
            desc += txt;
            desc += "m/s\n";

            desc += Game::GetText("tacref.range");
            desc += "\t";

            FormatNumber(txt, design->max_range);
            desc += txt;
            desc += "m\n";

            desc += Game::GetText("tacref.damage");
            desc += "\t";

            if (design->damage > 0) {
                FormatNumber(txt, design->damage * design->charge);
                desc += txt;
                if (design->beam)
                    desc += "/s";
            }
            else {
                desc += Game::GetText("tacref.none");
            }

            desc += "\n";

            if (!design->primary && design->damage > 0) {
                desc += Game::GetText("tacref.kill-radius");
                desc += "\t";
                FormatNumber(txt, design->lethal_radius);
                desc += txt;
                desc += " m\n";
            }

            TxtStats->SetText(FText::FromString(FString(desc.data())));
        }
    }

    if (TxtDescription) {
        if (design && design->description.length()) {
            TxtDescription->SetText(FText::FromString(FString(design->description.data())));
        }
        else {
            TxtDescription->SetText(FText::FromString(FString(Game::GetText("tacref.no-info").data())));
        }
    }
}

// +--------------------------------------------------------------------+

void UTacRefDlg::ExecFrame()
{
    // Legacy dialog had no per-frame logic besides camera manipulation.
    // Mouse input should be routed via UMG input handlers or PlayerController.
}

// +--------------------------------------------------------------------+

bool UTacRefDlg::SetCaptureBeauty()
{
    // Legacy used EventDispatch::CaptureMouse(beauty).
    // In UE, capture is typically handled via Slate/UMG overrides.
    // Keep as compatibility stub.
    return false;
}

bool UTacRefDlg::ReleaseCaptureBeauty()
{
    return false;
}

// +--------------------------------------------------------------------+

void UTacRefDlg::UpdateAzimuth(double a)
{
    cam_az += a;

    if (cam_az > PI)
        cam_az = -2 * PI + cam_az;
    else if (cam_az < -PI)
        cam_az = 2 * PI + cam_az;
}

void UTacRefDlg::UpdateElevation(double e)
{
    cam_el += e;

    const double limit = (0.43 * PI);

    if (cam_el > limit)
        cam_el = limit;
    else if (cam_el < -limit)
        cam_el = -limit;
}

void UTacRefDlg::UpdateZoom(double delta)
{
    cam_zoom *= delta;

    if (cam_zoom < 1.2)
        cam_zoom = 1.2;
    else if (cam_zoom > 10.0)
        cam_zoom = 10.0;
}

void UTacRefDlg::UpdateCamera()
{
    // Legacy camera math:
    const double x = cam_zoom * radius * sin(cam_az) * cos(cam_el);
    const double y = cam_zoom * radius * cos(cam_az) * cos(cam_el);
    const double z = cam_zoom * radius * sin(cam_el);

    // Original: cam.LookAt(Point(0,0,0), Point(x,z,y), Point(0,1,0));
    // Unreal note: if you are driving a SceneCapture/preview actor camera,
    // build a FVector from the legacy axis mapping (x, z, y):
    const FVector Eye((float)x, (float)z, (float)y);
    const FVector At(0.0f, 0.0f, 0.0f);
    const FVector Up(0.0f, 1.0f, 0.0f);

    // Hook your preview camera update here:
    // UE_LOG(LogTacRefDlg, Verbose, TEXT("UpdateCamera Eye=(%f,%f,%f)"), Eye.X, Eye.Y, Eye.Z);
    (void)Eye; (void)At; (void)Up;
}

// +--------------------------------------------------------------------+
// Button handlers (UMG)
// +--------------------------------------------------------------------+

void UTacRefDlg::HandleShipsClicked()
{
    if (mode == MODE_SHIPS)
        return;

    mode = MODE_SHIPS;

    // NOTE: UListView uses UObject items; the legacy ListBox stored raw pointers.
    // Keep a native array of pointers and populate a UListView with wrapper objects.
    // This file preserves selection logic as comments for your wrapper integration.

    // Swap preview to 3D ship mode:
    UpdateCamera();

    if (BtnShips) BtnShips->SetIsEnabled(false);
    if (BtnWeaps) BtnWeaps->SetIsEnabled(true);

    // TODO: select ship_index if list populated, then SelectShip(pointer).
}

void UTacRefDlg::HandleWeaponsClicked()
{
    if (mode == MODE_WEAPONS)
        return;

    mode = MODE_WEAPONS;

    if (BtnShips) BtnShips->SetIsEnabled(true);
    if (BtnWeaps) BtnWeaps->SetIsEnabled(false);

    // TODO: populate weapon list via wrapper items and SelectWeapon(pointer).
}

void UTacRefDlg::HandleCloseClicked()
{
    if (manager)
        manager->ShowMenuDlg();
}

// +--------------------------------------------------------------------+
// Legacy compatibility stubs (not used by UMG directly)
// +--------------------------------------------------------------------+

void UTacRefDlg::OnClose(void* Event)
{
    (void)Event;
    HandleCloseClicked();
}

void UTacRefDlg::OnSelect(void* Event)
{
    (void)Event;

    // Legacy selected item from ListBox and cast DWORD to ShipDesign/WeaponDesign.
    // With UListView, use a wrapper UObject holding an index or pointer.
}

void UTacRefDlg::OnMode(void* Event)
{
    (void)Event;
    // Use HandleShipsClicked / HandleWeaponsClicked from button bindings.
}

void UTacRefDlg::OnCamRButtonDown(void* Event) { (void)Event; }
void UTacRefDlg::OnCamRButtonUp(void* Event) { (void)Event; }
void UTacRefDlg::OnCamMove(void* Event) { (void)Event; }
void UTacRefDlg::OnCamZoom(void* Event) { (void)Event; }
