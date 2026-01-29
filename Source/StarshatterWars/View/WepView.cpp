/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         WepView.cpp
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO:
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    View class for Tactical HUD Overlay
*/

#include "WepView.h"

#include "HUDView.h"
#include "HUDSounds.h"
#include "Ship.h"
#include "Computer.h"
#include "NavSystem.h"
#include "Drive.h"
#include "Power.h"
#include "Shield.h"
#include "SimContact.h"
#include "ShipDesign.h"
#include "SimShot.h"
#include "Drone.h"
#include "Weapon.h"
#include "Sim.h"
#include "StarSystem.h"
#include "WeaponGroup.h"

#include "CameraView.h"
#include "Color.h"
#include "Window.h"
#include "Video.h"
#include "Screen.h"
#include "DataLoader.h"
#include "SimScene.h"
#include "FontManager.h"
#include "Graphic.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Game.h"
#include "FormatUtil.h"

#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogWepView, Log, All);

// +--------------------------------------------------------------------+
// Bitmap -> UTexture2D* (assets now managed as Unreal textures)

class UTexture2D;

static UTexture2D* tac_left = nullptr;
static UTexture2D* tac_right = nullptr;
static UTexture2D* tac_button = nullptr;
static UTexture2D* tac_man = nullptr;
static UTexture2D* tac_aut = nullptr;
static UTexture2D* tac_def = nullptr;

static BYTE* tac_left_shade = nullptr;
static BYTE* tac_right_shade = nullptr;
static BYTE* tac_button_shade = nullptr;
static BYTE* tac_man_shade = nullptr;
static BYTE* tac_aut_shade = nullptr;
static BYTE* tac_def_shade = nullptr;



// +--------------------------------------------------------------------+

WepView* WepView::wep_view = nullptr;

// +--------------------------------------------------------------------+

WepView::WepView(Window* c)
    : View(c), sim(nullptr), ship(nullptr), target(nullptr), active_region(nullptr),
    transition(false), mode(0), mouse_down(0)
{
    wep_view = this;

    sim = Sim::GetSim();

    // NOTE:
    // Original code prepared PCX bitmaps and per-pixel "shade" buffers, then drew them via Window.
    // In Unreal, these should be migrated to UTexture2D assets (and ideally Slate/UMG drawing).
    // The existing HUDView::PrepareBitmap / ColorizeBitmap calls are no longer valid with UTexture2D*.
    // Leaving these as TODO hooks to keep behavior equivalent without bloating this file with loader code.
    //
    // TODO (Unreal):
    // - Load textures: "TAC_left", "TAC_right", "TAC_button", "MAN", "AUTO", "DEF"
    // - Replace shade-buffer pipeline with material tinting or Slate color modulation
    UE_LOG(LogWepView, Verbose, TEXT("WepView constructed: texture-backed overlay assets are TODO for Unreal."));

    OnWindowMove();

    HudFont = FontManager::Find("HUD");
    BigFont = FontManager::Find("GUI");

    hud = HUDView::GetInstance();
    if (hud)
        SetColor(hud->GetHUDColor());
}

WepView::~WepView()
{
    // NOTE:
    // Texture lifetimes are expected to be owned/managed by Unreal's asset system/GC.
    // Do not manually delete UTexture2D* here.

    delete[] tac_left_shade;
    delete[] tac_right_shade;
    delete[] tac_button_shade;
    delete[] tac_man_shade;
    delete[] tac_aut_shade;
    delete[] tac_def_shade;

    tac_left_shade = nullptr;
    tac_right_shade = nullptr;
    tac_button_shade = nullptr;
    tac_man_shade = nullptr;
    tac_aut_shade = nullptr;
    tac_def_shade = nullptr;

    wep_view = nullptr;
}

void
WepView::OnWindowMove()
{
    width = window->Width();
    height = window->Height();
    xcenter = (width / 2.0) - 0.5;
    ycenter = (height / 2.0) + 0.5;

    int btn_loc = width / 2 - 147 - 45;
    int man_loc = width / 2 - 177 - 16;
    int aut_loc = width / 2 - 145 - 16;
    int def_loc = width / 2 - 115 - 16;

    int index = 0;

    for (int i = 0; i < MAX_WEP; i++) {
        btn_rect[index++] = Rect(btn_loc, 30, 90, 20);
        btn_rect[index++] = Rect(man_loc, 56, 32, 8);
        btn_rect[index++] = Rect(aut_loc, 56, 32, 8);
        btn_rect[index++] = Rect(def_loc, 56, 32, 8);

        btn_loc += 98;
        man_loc += 98;
        aut_loc += 98;
        def_loc += 98;
    }
}

// +--------------------------------------------------------------------+

bool
WepView::Update(SimObject* obj)
{
    if (obj == ship) {
        ship = nullptr;
        target = nullptr;
    }
    else if (obj == target) {
        target = nullptr;
    }

    return SimObserver::Update(obj);
}

const char*
WepView::GetObserverName() const
{
    return "WepView";
}

// +--------------------------------------------------------------------+

void
WepView::Refresh()
{
    sim = Sim::GetSim();
    if (!sim || !hud || hud->GetHUDMode() == EHUDMode::Off)
        return;

    if (ship != sim->GetPlayerShip()) {
        ship = sim->GetPlayerShip();

        if (ship) {
            if (ship->Life() == 0 || ship->IsDying() || ship->IsDead()) {
                ship = nullptr;
            }
            else {
                Observe(ship);
            }
        }
    }

    if (mode < 1)
        return;

    if (ship) {
        // no tactical overlay for fighters:
        if (ship->Design() && !ship->Design()->wep_screen) {
            mode = 0;
            return;
        }

        // no hud in transition:
        if (ship->InTransition()) {
            transition = true;
            return;
        }
        else if (transition) {
            transition = false;
            RestoreOverlay();
        }

        if (target != ship->GetTarget()) {
            target = ship->GetTarget();
            if (target) Observe(target);
        }

        DrawOverlay();
    }
    else {
        if (target) {
            target = nullptr;
        }
    }
}

// +--------------------------------------------------------------------+

void
WepView::ExecFrame()
{
    int hud_mode = 1;

    // update the position of HUD elements that are
    // part of the 3D scene (like fpm and lcos sprites)

    if (hud) {
        if (HudColor != hud->GetHUDColor()) {
            HudColor = hud->GetHUDColor();
            SetColor(HudColor);
        }

        if (hud->GetHUDMode() == EHUDMode::Off)
            hud_mode = 0;
    }

    if (ship && !transition && mode > 0 && hud_mode > 0) {
        if (mode > 0) {
            DoMouseFrame();
        }
    }
}

// +--------------------------------------------------------------------+

void
WepView::SetOverlayMode(int m)
{
    if (mode != m) {
        mode = m;

        if (hud)
            hud->SetOverlayMode(mode);

        RestoreOverlay();
    }
}

void
WepView::CycleOverlayMode()
{
    SetOverlayMode(!mode);
}

void
WepView::RestoreOverlay()
{
    if (mode > 0) {
        HUDSounds::PlaySound(HUDSounds::SND_WEP_DISP);
    }
    else {
        HUDSounds::PlaySound(HUDSounds::SND_WEP_MODE);
    }
}

// +--------------------------------------------------------------------+

void
WepView::SetColor(FColor c)
{
    HUDView* hud = HUDView::GetInstance();

    if (hud) {
        HudColor = hud->GetHUDColor();
        TextColor = hud->GetTextColor();
    }
    else {
        HudColor = c;
        TextColor = c;
    }

    // NOTE:
    // Original code recolored bitmaps using per-pixel shade buffers.
    // With UTexture2D*, prefer tinting via material params or Slate color.
    // TODO (Unreal): apply tint to overlay textures/materials.
}

// +--------------------------------------------------------------------+

void
WepView::DrawOverlay()
{
    // NOTE:
    // The original drew Bitmap resources through Window::DrawBitmap/FadeBitmap.
    // With Unreal textures, this needs a rendering path (Slate/UMG/HUD canvas).
    // Keeping gameplay logic intact; rendering calls are TODO.
    //
    // TODO (Unreal):
    // - Draw tac_left/tac_right at top center
    // - Draw tac_button and MAN/AUTO/DEF toggles
    // - Draw subtarget status text

    if (!ship)
        return;

    // Preserve the original decision logic for subtarget text and status color:
    Rect tgt_rect;
    tgt_rect.x = width / 2 + 73;
    tgt_rect.y = 74;
    tgt_rect.w = 100;
    tgt_rect.h = 15;

    Text           subtxt;
    FColor         stat = HudColor;
    static DWORD   blink = Game::RealTime();

    if (ship->GetTarget()) {
        if (ship->GetSubTarget()) {
            int blink_delta = Game::RealTime() - blink;

            SimSystem* sys = ship->GetSubTarget();
            subtxt = sys->Abbreviation();
            switch (sys->GetStatus()) {
            case SYSTEM_STATUS::DEGRADED:
                stat = FColor(255, 255, 0); 
                break;
            case SYSTEM_STATUS::CRITICAL: 
                stat = FColor(255, 180, 0);
                break;
            case SYSTEM_STATUS::DESTROYED: 
                stat = FColor(255, 0, 0);
                break;
            case SYSTEM_STATUS::MAINT:
                if (blink_delta < 250)
                    stat = FColor(8, 8, 8);
                break;
            }

            if (blink_delta > 500)
                blink = Game::RealTime();
        }
        else {
            subtxt = ship->GetTarget()->Name();
        }
    }
    else {
        subtxt = "NO TGT";
    }

    subtxt.toUpper();

    // If you still route text through Window/Font, keep that pipeline:
    if (hud_font && window) {
        hud_font->SetColor(stat);
        window->SetFont(hud_font);
        window->DrawText(subtxt.data(), subtxt.length(), tgt_rect, DT_SINGLELINE | DT_CENTER);
    }

    UE_LOG(LogWepView, VeryVerbose, TEXT("WepView::DrawOverlay called; full Unreal texture rendering is TODO."));
}

// +--------------------------------------------------------------------+

void
WepView::DoMouseFrame()
{
    static int mouse_down_local = false;
    static int mouse_down_x = 0;
    static int mouse_down_y = 0;

    int x = Mouse::X();
    int y = Mouse::Y();

    // coarse-grained test: is mouse in overlay at all?
    if (x < width / 2 - 256 || x > width / 2 + 256 || y > 90) {
        mouse_in = false;
        return;
    }

    mouse_in = true;

    if (Mouse::LButton()) {
        if (!mouse_down_local) {
            mouse_down_local = true;
            mouse_down_x = x;
            mouse_down_y = y;
        }

        // check weapons buttons:
        int max_wep = ship->Weapons().size();

        if (max_wep > MAX_WEP)
            max_wep = MAX_WEP;

        for (int i = 0; i < max_wep; i++) {
            int index = i * 4;

            if (CheckButton(index, mouse_down_x, mouse_down_y)) {
                ship->FireWeapon(i);
                return;
            }
            else if (CheckButton(index + 1, mouse_down_x, mouse_down_y)) {
                ship->Weapons()[i]->SetFiringOrders(Weapon::MANUAL);
                return;
            }
            else if (CheckButton(index + 2, mouse_down_x, mouse_down_y)) {
                ship->Weapons()[i]->SetFiringOrders(Weapon::AUTO);
                return;
            }
            else if (CheckButton(index + 3, mouse_down_x, mouse_down_y)) {
                ship->Weapons()[i]->SetFiringOrders(Weapon::POINT_DEFENSE);
                return;
            }
        }
    }
    else if (mouse_down_local) {
        mouse_down_local = false;
        mouse_down_x = 0;
        mouse_down_y = 0;

        // check subtarget buttons:
        if (ship->GetTarget()) {
            Rect r(width / 2 + 50, 70, 20, 20);
            if (r.Contains(x, y)) {
                CycleSubTarget(-1);
                return;
            }

            r.x = width / 2 + 180;
            if (r.Contains(x, y)) {
                CycleSubTarget(1);
                return;
            }
        }
    }
}

bool
WepView::CheckButton(int index, int x, int y)
{
    if (index >= 0 && index < MAX_BTN) {
        return btn_rect[index].Contains(x, y) ? true : false;
    }

    return false;
}

void
WepView::CycleSubTarget(int direction)
{
    if (ship->GetTarget() == nullptr || ship->GetTarget()->Type() != SimObject::SIM_SHIP)
        return;

    ship->CycleSubTarget(direction);
}

bool
WepView::IsMouseLatched()
{
    return mouse_in;
}

