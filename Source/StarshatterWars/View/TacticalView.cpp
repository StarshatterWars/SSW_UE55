/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright 2025-2026. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         TacticalView.cpp
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR: John DiCamillo
    ORIGINAL STUDIO: Destroyer Studios LLC

    OVERVIEW
    ========
    View class for Tactical Data Readout HUD Overlay
*/

#include "TacticalView.h"
#include "CoreMinimal.h"          // UE_LOG, FVector
#include "Math/Vector.h"          // FVector

#include "QuantumView.h"
#include "RadioView.h"
#include "RadioMessage.h"
#include "RadioTraffic.h"
#include "HUDSounds.h"
#include "HUDView.h"
#include "WepView.h"
#include "CameraManager.h"
#include "Ship.h"
#include "ShipManager.h"
#include "ShipDesign.h"
#include "QuantumDrive.h"
#include "Farcaster.h"
#include "Instruction.h"
#include "SimElement.h"
#include "SimContact.h"
#include "Sim.h"
#include "Starshatter.h"
#include "GameScreen.h"
#include "MenuView.h"
#include "GameStructs.h"

#include "SimProjector.h"
#include "Color.h"
#include "Window.h"
#include "Video.h"
#include "DataLoader.h"
#include "SimScene.h"
#include "FontManager.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "MouseController.h"
#include "Menu.h"
#include "Game.h"
#include "FormatUtil.h"

// +--------------------------------------------------------------------+

TacticalView* TacticalView::tac_view = 0;

// +--------------------------------------------------------------------+

TacticalView::TacticalView(Window* c, UGameScreen* parent)
    : View(c),
    gamescreen(parent),
    ship(0),
    camview(0),
    projector(0),
    mouse_down(0),
    right_down(0),
    shift_down(0),
    show_move(0),
    show_action(0),
    active_menu(0),
    menu_view(0),
    msg_ship(0),
    base_alt(0),
    move_alt(0)
{
    tac_view = this;
    sim = Sim::GetSim();

    width = window->Width();
    height = window->Height();
    xcenter = (width / 2.0) - 0.5;
    ycenter = (height / 2.0) + 0.5;
    font = FontManager::Find("HUD");

    SetColor(FColor::White);

    mouse_start.X = 0;
    mouse_start.Y = 0;
    mouse_action.X = 0;
    mouse_action.Y = 0;

    menu_view = new MenuView(window);
}

TacticalView::~TacticalView()
{
    delete menu_view;
    tac_view = 0;
}

void TacticalView::OnWindowMove()
{
    width = window->Width();
    height = window->Height();
    xcenter = (width / 2.0) - 0.5;
    ycenter = (height / 2.0) + 0.5;

    if (menu_view)
        menu_view->OnWindowMove();
}

// +--------------------------------------------------------------------+

bool TacticalView::Update(SimObject* obj)
{
    if (obj == ship) {
        ship = 0;
    }

    if (obj == msg_ship) {
        msg_ship = 0;
    }

    return SimObserver::Update(obj);
}

const char* TacticalView::GetObserverName() const
{
    return "TacticalView";
}

void TacticalView::UseProjector(SimProjector* p)
{
    projector = p;
}

// +--------------------------------------------------------------------+

void TacticalView::Refresh()
{
    sim = Sim::GetSim();

    if (sim) {
        bool rebuild = false;

        if (ship != sim->GetPlayerShip()) {
            ship = sim->GetPlayerShip();

            if (ship) {
                if (ship->Life() == 0 || ship->IsDying() || ship->IsDead()) {
                    ship = 0;
                }
                else {
                    Observe(ship);
                }
            }

            rebuild = true;
        }

        if (ship) {
            if (current_sector != ship->GetRegion()->GetName())
                rebuild = true;

            if (rebuild) {
                BuildMenu();
                current_sector = ship->GetRegion()->GetName();
            }
        }
    }

    if (!ship || ship->InTransition())
        return;

    DrawMouseRect();

    if (sim) {
        ListIter<Ship> sel = sim->GetSelection();

        if (sel.size()) {
            while (++sel) {
                Ship* selection = sel.value();

                // draw selection rect on selected ship:
                if (selection && selection->Rep())
                    DrawSelection(selection);
            }

            RadioView* rv = RadioView::GetInstance();
            QuantumView* qv = QuantumView::GetInstance();

            if ((!rv || !rv->IsMenuShown()) && (!qv || !qv->IsMenuShown())) {
                sel.reset();

                if (sel.size() == 1) {
                    DrawSelectionInfo(sel.next());
                }
                else {
                    DrawSelectionList(sel);
                }
            }
        }
    }

    DrawMenu();

    if (show_move) {
        Mouse::Show(false);
        DrawMove();
    }
    else if (show_action) {
        Mouse::Show(false);
        DrawAction();
    }
}

// +--------------------------------------------------------------------+

void TacticalView::ExecFrame()
{
    HUDView* hud = HUDView::GetInstance();
    if (hud) {
        if (HudColor != hud->GetTextColor()) {
            HudColor = hud->GetTextColor();
            SetColor(HudColor);
        }
    }
}

// +--------------------------------------------------------------------+

void TacticalView::SetColor(FColor c)
{
    HUDView* hud = HUDView::GetInstance();

    if (hud) {
        HudColor = hud->GetHUDColor();
        HudColor = hud->GetTextColor();
    }
    else {
        HudColor = c;
        TextColor = c;
    }
}

// +--------------------------------------------------------------------+

void TacticalView::DrawMouseRect()
{
    if (mouse_rect.w > 0 && mouse_rect.h > 0) {
       
       FColor c(
            FMath::Clamp(int(hud_color.R * 0.66f), 0, 255),
            FMath::Clamp(int(hud_color.G * 0.66f), 0, 255),
            FMath::Clamp(int(hud_color.B * 0.66f), 0, 255),
            hud_color.A
        );

        if (shift_down)
            c = FColor::Orange;

        window->DrawRect(mouse_rect, c);
    }
}

// +--------------------------------------------------------------------+

void TacticalView::DrawSelection(Ship* seln)
{
    if (!seln)
        return;

    Graphic* g = seln->Rep();
    Rect r = g->ScreenRect();

    FVector mark_pt = seln->Location(); // was: Point

    projector->Transform(mark_pt);

    // clip:
    if (mark_pt.Z > 1.0) {
        projector->Project(mark_pt);

        int x = (int)mark_pt.X;
        int y = r.y;

        if (y >= 2000)
            y = (int)mark_pt.Y;

        if (x > 4 && x < width - 4 && y > 4 && y < height - 4) {

            const int BAR_LENGTH = 40;

            // life bars:
            int sx = x - BAR_LENGTH / 2;
            int sy = y - 8;

            double hull_strength = seln->HullStrength() / 100.0;

            int hw = (int)(BAR_LENGTH * hull_strength);
            int sw = (int)(BAR_LENGTH * (seln->ShieldStrength() / 100.0));

            if (hw < 0)
                hw = 0;
            if (sw < 0)
                sw = 0;

            SYSTEM_STATUS s = SYSTEM_STATUS::NOMINAL;

            if (hull_strength < 0.30)
                s = SYSTEM_STATUS::CRITICAL;
            else if (hull_strength < 0.60)
                s = SYSTEM_STATUS::DEGRADED;

            FColor hc = HUDView::GetStatusColor(s);
            FColor sc = hud_color;

            window->FillRect(sx, sy, sx + hw, sy + 1, hc);
            window->FillRect(sx, sy + 3, sx + sw, sy + 4, sc);
        }
    }
}

// +--------------------------------------------------------------------+

void TacticalView::DrawSelectionInfo(Ship* seln)
{
    if (!ship || !seln)
        return;

    Rect label_rect(width - 140, 10, 90, 12);
    Rect info_rect(width - 100, 10, 90, 12);

    if (width >= 800) {
        label_rect.x -= 20;
        info_rect.x -= 20;
        info_rect.w += 20;
    }

    static char name[64];
    static char design[64];
    static char shield[32];
    static char hull[32];
    static char range[32];
    static char heading[32];
    static char speed[32];
    static char orders[64];
    static char psv[32];
    static char act[32];

    int show_labels = width > 640;
    int full_info = true;
    int shield_val = seln->ShieldStrength();
    int hull_val = seln->HullStrength();

    if (shield_val < 0)
        shield_val = 0;
    if (hull_val < 0)
        hull_val = 0;

    sprintf_s(name, "%s", seln->Name());

    if (show_labels) {
        sprintf_s(shield, "%s %03d", Game::GetText("HUDView.symbol.shield").data(), shield_val);
        sprintf_s(hull, "%s %03d", Game::GetText("HUDView.symbol.hull").data(), hull_val);
    }
    else {
        sprintf_s(shield, "%03d", shield_val);
        sprintf_s(hull, "%03d", hull_val);
    }

    // was: Point(seln->Location()-ship->Location()).length()/1000
    const FVector RangeVec = seln->Location() - ship->Location();
    FormatNumberExp(range, RangeVec.Size() / 1000.0);
    strcat_s(range, " km");

    sprintf_s(heading, "%03d %s", (int)(seln->CompassHeading() / DEGREES),
        Game::GetText("HUDView.symbol.degrees").data());

    double ss = seln->Velocity().Size();
    if (FVector::DotProduct(seln->Velocity(), seln->Heading()) < 0)
        ss = -ss;

    FormatNumberExp(speed, ss);
    strcat_s(speed, " m/s");

    SimContact* contact = 0;

    // always recognize ownside:
    if (seln->GetIFF() != ship->GetIFF()) {
        ListIter<SimContact> c = ship->ContactList();
        while (++c) {
            if (c->GetShip() == seln) {
                contact = c.value();
                if (c->GetIFF(ship) > seln->GetIFF()) {
                    sprintf_s(name, "%s %04d", Game::GetText("TacView.contact").data(), seln->GetContactID());
                    full_info = false;
                }

                break;
            }
        }
    }

    if (show_labels) {
        font->SetColor(txt_color);
        font->SetAlpha(1);

        font->DrawText(Game::GetText("TacView.name"), 5, label_rect, DT_LEFT);
        label_rect.y += 10;
        font->DrawText(Game::GetText("TacView.type"), 5, label_rect, DT_LEFT);
        label_rect.y += 10;

        if (full_info) {
            font->DrawText(Game::GetText("TacView.shield"), 5, label_rect, DT_LEFT);
            label_rect.y += 10;
            font->DrawText(Game::GetText("TacView.hull"), 5, label_rect, DT_LEFT);
            label_rect.y += 10;
        }

        font->DrawText(Game::GetText("TacView.range"), 4, label_rect, DT_LEFT);
        label_rect.y += 10;

        if (full_info) {
            font->DrawText(Game::GetText("TacView.speed"), 4, label_rect, DT_LEFT);
            label_rect.y += 10;
            font->DrawText(Game::GetText("TacView.heading"), 4, label_rect, DT_LEFT);
            label_rect.y += 10;
        }
        else {
            font->DrawText(Game::GetText("TacView.passive"), 4, label_rect, DT_LEFT);
            label_rect.y += 10;
            font->DrawText(Game::GetText("TacView.active"), 4, label_rect, DT_LEFT);
            label_rect.y += 10;
        }
    }

    font->DrawText(name, 0, info_rect, DT_LEFT);
    info_rect.y += 10;

    if (full_info) {
        sprintf_s(design, "%s %s", seln->Abbreviation(), seln->Design()->display_name);
        font->DrawText(design, 0, info_rect, DT_LEFT);
        info_rect.y += 10;
    }
    else {
        if (seln->IsStarship())
            font->DrawText(Game::GetText("TacView.starship"), 8, info_rect, DT_LEFT);
        else
            font->DrawText(Game::GetText("TacView.fighter"), 7, info_rect, DT_LEFT);

        info_rect.y += 10;
    }

    if (full_info) {
        font->DrawText(shield, 0, info_rect, DT_LEFT);
        info_rect.y += 10;

        font->DrawText(hull, 0, info_rect, DT_LEFT);
        info_rect.y += 10;
    }

    font->DrawText(range, 0, info_rect, DT_LEFT);
    info_rect.y += 10;

    if (full_info) {
        font->DrawText(speed, 0, info_rect, DT_LEFT);
        info_rect.y += 10;

        font->DrawText(heading, 0, info_rect, DT_LEFT);
        info_rect.y += 10;

        if (seln->GetIFF() == ship->GetIFF()) {
            Instruction* instr = seln->GetRadioOrders();
            if (instr && instr->Action()) {
                strcpy_s(orders, RadioMessage::ActionName(instr->Action()));

                if (instr->Action() == RadioMessage::QUANTUM_TO) {
                    strcat_s(orders, " ");
                    strcat_s(orders, instr->RegionName());
                }
            }
            else {
                *orders = 0;
            }

            if (*orders) {
                if (show_labels) {
                    font->DrawText(Game::GetText("TacView.orders"), 5, label_rect, DT_LEFT);
                    label_rect.y += 10;
                }

                font->DrawText(orders, 0, info_rect, DT_LEFT);
                info_rect.y += 10;
            }
        }
    }
    else {
        sprintf_s(psv, "%03d", (int)(contact->PasReturn() * 100.0));
        sprintf_s(act, "%03d", (int)(contact->ActReturn() * 100.0));

        if (contact->Threat(ship))
            strcat_s(psv, " !");

        font->DrawText(psv, 0, info_rect, DT_LEFT);
        info_rect.y += 10;
        font->DrawText(act, 0, info_rect, DT_LEFT);
        info_rect.y += 10;
    }

    // Print-style debug removed; use UE_LOG if/when re-enabled.
    // UE_LOG(LogTemp, Verbose, TEXT("DirectorInfo: %s"), ANSI_TO_TCHAR(seln->GetDirectorInfo()));
}

// +--------------------------------------------------------------------+

void TacticalView::DrawSelectionList(ListIter<Ship> seln)
{
    int index = 0;
    Rect info_rect(width - 100, 10, 90, 12);

    while (++seln) {
        char name[64];
        sprintf_s(name, "%s", seln->Name());

        // always recognize ownside:
        if (seln->GetIFF() != ship->GetIFF()) {
            ListIter<SimContact> c = ship->ContactList();
            while (++c) {
                if (c->GetShip() == seln.value()) {
                    if (c->GetIFF(ship) > seln->GetIFF()) {
                        sprintf_s(name, "%s %04d", Game::GetText("TacView.contact").data(), seln->GetContactID());
                    }

                    break;
                }
            }
        }

        font->DrawText(name, 0, info_rect, DT_LEFT);
        info_rect.y += 10;
        index++;

        if (index >= 10)
            break;
    }
}

// +--------------------------------------------------------------------+

void
TacticalView::DoMouseFrame()
{
    static DWORD rbutton_latch = 0;

    Starshatter* stars = Starshatter::GetInstance();
    if (stars->InCutscene())
        return;

    MouseController* LocalMouseController = MouseController::GetInstance();

    if (Mouse::RButton()) {
        if (!right_down && (!LocalMouseController || !LocalMouseController->Active())) {
            rbutton_latch = Game::RealTime();
            right_down = true;
        }
    }
    else {
        if (sim && right_down && (Game::RealTime() - rbutton_latch < 250)) {
            Ship* seln = WillSelectAt(Mouse::X(), Mouse::Y());

            if (seln && sim->IsSelected(seln) &&
                seln->GetIFF() == ship->GetIFF() &&
                ship->GetElement()->CanCommand(seln->GetElement())) {

                msg_ship = seln;
                Observe(msg_ship);
            }
            else if (ship && seln == ship &&
                (!ship->GetDirector() ||
                    ship->GetDirector()->Type() != ShipManager::DIR_TYPE)) {

                msg_ship = seln;
            }
            else {
                msg_ship = nullptr;
            }
        }

        right_down = false;
    }

    if (menu_view)
        menu_view->DoMouseFrame();

    // Re-use the same local controller
    if (!LocalMouseController || !LocalMouseController->Active()) {

        if (Mouse::LButton()) {
            if (!mouse_down) {
                mouse_start.X = Mouse::X();
                mouse_start.Y = Mouse::Y();
                shift_down = Keyboard::KeyDown(VK_SHIFT);
            }
            else {
                if (Mouse::X() < mouse_start.X) {
                    mouse_rect.x = Mouse::X();
                    mouse_rect.w = mouse_start.X - Mouse::X();
                }
                else {
                    mouse_rect.x = mouse_start.X;
                    mouse_rect.w = Mouse::X() - mouse_start.X;
                }

                if (Mouse::Y() < mouse_start.Y) {
                    mouse_rect.y = Mouse::Y();
                    mouse_rect.h = mouse_start.Y - Mouse::Y();
                }
                else {
                    mouse_rect.y = mouse_start.Y;
                    mouse_rect.h = Mouse::Y() - mouse_start.Y;
                }

                if (Mouse::RButton() || show_move || show_action) {
                    mouse_rect.w = 0;
                    mouse_rect.h = 0;
                }
                else {
                    SelectRect(mouse_rect);
                }
            }

            mouse_down = true;
        }
        else {
            if (mouse_down) {
                const int mouse_x = Mouse::X();
                const int mouse_y = Mouse::Y();

                if (menu_view && menu_view->GetAction()) {
                    ProcessMenuItem(menu_view->GetAction());
                    Mouse::Show(true);
                }
                else if (show_move) {
                    SendMove();
                    show_move = false;
                    Mouse::Show(true);
                }
                else if (show_action) {
                    SendAction();
                    show_action = false;
                    Mouse::Show(true);
                }
                else if (!HUDView::IsMouseLatched() && !WepView::IsMouseLatched()) {

                    const int dx = FMath::Abs(mouse_x - mouse_start.X);
                    const int dy = FMath::Abs(mouse_y - mouse_start.Y);

                    static DWORD click_time = 0;

                    if (dx < 3 && dy < 3) {
                        const bool hit = SelectAt(mouse_x, mouse_y);

                        if (ship->IsStarship() && Game::RealTime() - click_time < 350)
                            SetHelm(hit);

                        click_time = Game::RealTime();
                    }
                }

                mouse_rect = Rect();
                mouse_down = false;
            }
        }
    }

    if (show_action && !mouse_down && !right_down) {
        mouse_action.X = Mouse::X();
        mouse_action.Y = Mouse::Y();
    }
}

// +--------------------------------------------------------------------+

bool TacticalView::SelectAt(int x, int y)
{
    if (!ship)
        return false;

    Ship* selection = WillSelectAt(x, y);

    if (selection && shift_down)
        ship->SetTarget(selection);

    else if (sim && selection)
        sim->SetSelection(selection);

    return selection != 0;
}

// +--------------------------------------------------------------------+

bool TacticalView::SelectRect(const Rect& rect)
{
    bool result = false;

    if (!ship || !sim)
        return result;

    if (rect.w > 8 || rect.h > 8)
        sim->ClearSelection();

    // check distance to each contact:
    List<SimContact>& contact_list = ship->ContactList();

    for (int i = 0; i < ship->NumContacts(); i++) {
        Ship* test = contact_list[i]->GetShip();

        if (test && test != ship) {

            FVector test_loc = test->Location(); // was: Point
            projector->Transform(test_loc);

            if (test_loc.Z > 1) {
                projector->Project(test_loc);

                if (rect.Contains((int)test_loc.X, (int)test_loc.Y)) {
                    // shift-select targets:
                    if (shift_down) {
                        if (test->GetIFF() == 0 || test->GetIFF() == ship->GetIFF())
                            continue;

                        ship->SetTarget(test);
                        result = true;
                    }
                    else {
                        sim->AddSelection(test);
                        result = true;
                    }
                }
            }
        }
    }

    // select self only in orbit cam
    if (!shift_down && CameraManager::GetCameraMode() == CameraManager::MODE_ORBIT) {
        FVector test_loc = ship->Location(); // was: Point
        projector->Transform(test_loc);

        if (test_loc.Z > 1) {
            projector->Project(test_loc);

            if (rect.Contains((int)test_loc.X, (int)test_loc.Y)) {
                sim->AddSelection(ship);
                result = true;
            }
        }
    }

    return result;
}

// +--------------------------------------------------------------------+

Ship* TacticalView::WillSelectAt(int x, int y)
{
    Ship* selection = 0;

    if (ship) {
        // check distance to each contact:
        List<SimContact>& contact_list = ship->ContactList();

        for (int i = 0; i < ship->NumContacts(); i++) {
            Ship* test = contact_list[i]->GetShip();

            if (test) {
                // shift-select targets:
                if (shift_down) {
                    if (test->GetIFF() == 0 || test->GetIFF() == ship->GetIFF())
                        continue;
                }

                Graphic* g = test->Rep();
                if (g) {
                    Rect r = g->ScreenRect();

                    if (r.x == 2000 && r.y == 2000 && r.w == 0 && r.h == 0) {
                        if (projector) {
                            FVector loc = test->Location(); // was: Point
                            projector->Transform(loc);
                            projector->Project(loc);

                            r.x = (int)loc.X;
                            r.y = (int)loc.Y;
                        }
                    }

                    if (r.w < 20 || r.h < 20)
                        r.Inflate(20, 20);
                    else
                        r.Inflate(10, 10);

                    if (r.Contains(x, y)) {
                        selection = test;
                        break;
                    }
                }
            }
        }

        if (!selection && !shift_down) {
            Graphic* g = ship->Rep();
            if (g) {
                Rect r = g->ScreenRect();

                if (r.Contains(x, y)) {
                    selection = ship;
                }
            }
        }
    }

    if (selection == ship && CameraManager::GetCameraMode() != CameraManager::MODE_ORBIT)
        selection = 0;

    return selection;
}

// +--------------------------------------------------------------------+

void TacticalView::SetHelm(bool approach)
{
    FVector delta(0, 0, 0); // was: Point

    // double-click on ship: set helm to approach
    if (sim && approach) {
        ListIter<Ship> iter = sim->GetSelection();
        ++iter;
        Ship* selection = iter.value();

        if (selection != ship) {
            delta = selection->Location() - ship->Location();
            delta.Normalize();
        }
    }

    // double-click on space: set helm in direction
    if (delta.Size() < 1) {
        int mx = Mouse::X();
        int my = Mouse::Y();

        if (projector) {
            double focal_dist = width / tan(projector->XAngle());

            delta =
                projector->vpn() * focal_dist +
                projector->vup() * -1 * (my - height / 2) +
                projector->vrt() * (mx - width / 2);

            delta.Normalize();
        }
        else {
            return;
        }
    }

    double az = atan2(fabs(delta.X), delta.Z);
    double el = asin(delta.Y);

    if (delta.X < 0)
        az *= -1;

    az += PI;

    if (az >= 2 * PI)
        az -= 2 * PI;

    ship->SetHelmHeading(az);
    ship->SetHelmPitch(el);
}

// +====================================================================+
//
// TACTICAL COMMUNICATIONS MENU:
//

Menu* MainMenu = nullptr;
Menu* ViewMenu = nullptr;
Menu* EmconMenu = nullptr;

Menu* FighterMenu = nullptr;
Menu* StarshipMenu = nullptr;
Menu* ActionMenu = nullptr;
Menu* FormationMenu = nullptr;
Menu* SensorsMenu = nullptr;
Menu* QuantumMenu = nullptr;
Menu* FarcastMenu = nullptr;

static SimElement* dst_elem = 0;

enum VIEW_MENU {
    VIEW_FORWARD = 1000,
    VIEW_CHASE,
    VIEW_PADLOCK,
    VIEW_ORBIT,
    VIEW_NAV,
    VIEW_WEP,
    VIEW_ENG,
    VIEW_FLT,
    VIEW_INS,
    VIEW_CMD
};

const int QUANTUM = 2000;
const int FARCAST = 2001;

void TacticalView::Initialize()
{
    static int initialized = 0;
    if (initialized)
        return;

    ViewMenu = new Menu(Game::GetText("TacView.menu.view"));
    ViewMenu->AddItem(Game::GetText("TacView.item.forward"), VIEW_FORWARD);
    ViewMenu->AddItem(Game::GetText("TacView.item.chase"), VIEW_CHASE);
    ViewMenu->AddItem(Game::GetText("TacView.item.orbit"), VIEW_ORBIT);
    ViewMenu->AddItem(Game::GetText("TacView.item.padlock"), VIEW_PADLOCK);

    EmconMenu = new Menu(Game::GetText("TacView.menu.emcon"));

    QuantumMenu = new Menu(Game::GetText("TacView.menu.quantum"));
    FarcastMenu = new Menu(Game::GetText("TacView.menu.farcast"));

    MainMenu = new Menu(Game::GetText("TacView.menu.main"));

    ActionMenu = new Menu(Game::GetText("TacView.menu.action"));
    ActionMenu->AddItem(Game::GetText("TacView.item.engage"), RadioMessage::ATTACK);
    ActionMenu->AddItem(Game::GetText("TacView.item.bracket"), RadioMessage::BRACKET);
    ActionMenu->AddItem(Game::GetText("TacView.item.escort"), RadioMessage::ESCORT);
    ActionMenu->AddItem(Game::GetText("TacView.item.identify"), RadioMessage::IDENTIFY);
    ActionMenu->AddItem(Game::GetText("TacView.item.hold"), RadioMessage::WEP_HOLD);

    FormationMenu = new Menu(Game::GetText("TacView.menu.formation"));
    FormationMenu->AddItem(Game::GetText("TacView.item.diamond"), RadioMessage::GO_DIAMOND);
    FormationMenu->AddItem(Game::GetText("TacView.item.spread"), RadioMessage::GO_SPREAD);
    FormationMenu->AddItem(Game::GetText("TacView.item.box"), RadioMessage::GO_BOX);
    FormationMenu->AddItem(Game::GetText("TacView.item.trail"), RadioMessage::GO_TRAIL);

    SensorsMenu = new Menu(Game::GetText("TacView.menu.emcon"));
    SensorsMenu->AddItem(Game::GetText("TacView.item.emcon-1"), RadioMessage::GO_EMCON1);
    SensorsMenu->AddItem(Game::GetText("TacView.item.emcon-2"), RadioMessage::GO_EMCON2);
    SensorsMenu->AddItem(Game::GetText("TacView.item.emcon-3"), RadioMessage::GO_EMCON3);
    SensorsMenu->AddItem(Game::GetText("TacView.item.probe"), RadioMessage::LAUNCH_PROBE);

    FighterMenu = new Menu(Game::GetText("TacView.menu.context"));
    FighterMenu->AddMenu(Game::GetText("TacView.item.action"), ActionMenu);
    FighterMenu->AddMenu(Game::GetText("TacView.item.formation"), FormationMenu);
    FighterMenu->AddMenu(Game::GetText("TacView.item.sensors"), SensorsMenu);
    FighterMenu->AddItem(Game::GetText("TacView.item.patrol"), RadioMessage::MOVE_PATROL);
    FighterMenu->AddItem(Game::GetText("TacView.item.cancel"), RadioMessage::RESUME_MISSION);
    FighterMenu->AddItem("", 0);
    FighterMenu->AddItem(Game::GetText("TacView.item.rtb"), RadioMessage::RTB);
    FighterMenu->AddItem(Game::GetText("TacView.item.dock"), RadioMessage::DOCK_WITH);
    FighterMenu->AddMenu(Game::GetText("TacView.item.farcast"), FarcastMenu);

    StarshipMenu = new Menu(Game::GetText("TacView.menu.context"));
    StarshipMenu->AddMenu(Game::GetText("TacView.item.action"), ActionMenu);
    StarshipMenu->AddMenu(Game::GetText("TacView.item.sensors"), SensorsMenu);
    StarshipMenu->AddItem(Game::GetText("TacView.item.patrol"), RadioMessage::MOVE_PATROL);
    StarshipMenu->AddItem(Game::GetText("TacView.item.cancel"), RadioMessage::RESUME_MISSION);
    StarshipMenu->AddItem("", 0);
    StarshipMenu->AddMenu(Game::GetText("TacView.item.quantum"), QuantumMenu);
    StarshipMenu->AddMenu(Game::GetText("TacView.item.farcast"), FarcastMenu);

    initialized = 1;
}

void TacticalView::Close()
{
    delete ViewMenu;
    delete EmconMenu;
    delete MainMenu;
    delete FighterMenu;
    delete StarshipMenu;
    delete ActionMenu;
    delete FormationMenu;
    delete SensorsMenu;
    delete QuantumMenu;
    delete FarcastMenu;
}

// +--------------------------------------------------------------------+

void TacticalView::ProcessMenuItem(int action)
{
    Starshatter* stars = Starshatter::GetInstance();

    switch (action) {
    case RadioMessage::MOVE_PATROL:
        show_move = true;
        base_alt = 0;
        move_alt = 0;

        if (msg_ship)
            base_alt = msg_ship->Location().Y;
        break;

    case RadioMessage::ATTACK:
    case RadioMessage::BRACKET:
    case RadioMessage::ESCORT:
    case RadioMessage::IDENTIFY:
    case RadioMessage::DOCK_WITH:
        show_action = action;
        break;

    case RadioMessage::WEP_HOLD:
    case RadioMessage::RESUME_MISSION:
    case RadioMessage::RTB:
    case RadioMessage::GO_DIAMOND:
    case RadioMessage::GO_SPREAD:
    case RadioMessage::GO_BOX:
    case RadioMessage::GO_TRAIL:
    case RadioMessage::GO_EMCON1:
    case RadioMessage::GO_EMCON2:
    case RadioMessage::GO_EMCON3:
    case RadioMessage::LAUNCH_PROBE:
        if (msg_ship) {
            SimElement* elem = msg_ship->GetElement();
            RadioMessage* msg = new RadioMessage(elem, ship, action);
            if (msg)
                RadioTraffic::Transmit(msg);
        }
        else if (ship) {
            if (action == RadioMessage::GO_EMCON1)
                ship->SetEMCON(1);
            else if (action == RadioMessage::GO_EMCON2)
                ship->SetEMCON(2);
            else if (action == RadioMessage::GO_EMCON3)
                ship->SetEMCON(3);
            else if (action == RadioMessage::LAUNCH_PROBE)
                ship->LaunchProbe();
        }
        break;

    case VIEW_FORWARD: stars->PlayerCam(CameraManager::MODE_COCKPIT); break;
    case VIEW_CHASE:   stars->PlayerCam(CameraManager::MODE_CHASE);   break;
    case VIEW_PADLOCK: stars->PlayerCam(CameraManager::MODE_TARGET);  break;
    case VIEW_ORBIT:   stars->PlayerCam(CameraManager::MODE_ORBIT);   break;

    case VIEW_NAV: gamescreen->ShowNavDlg(); break;
    case VIEW_WEP: gamescreen->ShowWeaponsOverlay(); break;
    case VIEW_ENG: gamescreen->ShowEngDlg(); break;
    case VIEW_INS: HUDView::GetInstance()->CycleHUDInst(); break;
    case VIEW_FLT: gamescreen->ShowFltDlg(); break;

    case VIEW_CMD:
        if (ship && ship->IsStarship()) {
            ship->CommandMode();
        }
        break;

    case QUANTUM:
        if (sim) {
            Ship* s = msg_ship;

            if (!s)
                s = ship;

            if (s && s->GetQuantumDrive()) {
                QuantumDrive* quantum = s->GetQuantumDrive();
                if (quantum) {
                    MenuItem* menu_item = menu_view->GetMenuItem();
                    Text rgn_name = menu_item->GetText();
                    SimRegion* rgn = sim->FindRegion(rgn_name);

                    if (rgn) {
                        if (s == ship) {
                            quantum->SetDestination(rgn, FVector(0, 0, 0)); // was: Point
                            quantum->Engage();
                        }
                        else {
                            SimElement* elem = msg_ship->GetElement();
                            RadioMessage* msg = new RadioMessage(elem, ship, RadioMessage::QUANTUM_TO);
                            if (msg) {
                                msg->SetInfo(rgn_name);
                                RadioTraffic::Transmit(msg);
                            }
                        }
                    }
                }
            }
        }
        break;

    case FARCAST:
        if (sim && msg_ship) {
            MenuItem* menu_item = menu_view->GetMenuItem();
            Text rgn_name = menu_item->GetText();
            SimRegion* rgn = sim->FindRegion(rgn_name);

            if (rgn) {
                SimElement* elem = msg_ship->GetElement();
                RadioMessage* msg = new RadioMessage(elem, ship, RadioMessage::FARCAST_TO);
                if (msg) {
                    msg->SetInfo(rgn_name);
                    RadioTraffic::Transmit(msg);
                }
            }
        }
        break;

    default:
        break;
    }
}

// +--------------------------------------------------------------------+

void TacticalView::BuildMenu()
{
    if (MainMenu)    MainMenu->ClearItems();
    if (QuantumMenu) QuantumMenu->ClearItems();
    if (FarcastMenu) FarcastMenu->ClearItems();
    if (EmconMenu)   EmconMenu->ClearItems();

    if (!ship || !sim || !MainMenu || !QuantumMenu || !FarcastMenu || !EmconMenu)
        return;

    // ----------------------------------------------------------
    // PREPARE QUANTUM MENU (regions other than current, non-airspace)
    // ----------------------------------------------------------
    ListIter<SimRegion> rgn_iter = sim->GetRegions();
    while (++rgn_iter) {
        SimRegion* rgn = rgn_iter.value();

        if (rgn && rgn != ship->GetRegion() && rgn->GetType() != SimRegion::AIR_SPACE) {
            QuantumMenu->AddItem(rgn->GetName(), QUANTUM);
        }
    }

    // ----------------------------------------------------------
    // PREPARE FARCAST MENU (connected farcaster destinations)
    // ----------------------------------------------------------
    SimRegion* ship_rgn = ship->GetRegion();

    if (ship_rgn) {
        ListIter<Ship> ship_iter = ship_rgn->GetShips();
        while (++ship_iter) {
            Ship* s = ship_iter.value();
            if (!s)
                continue;

            Farcaster* farcaster = s->GetFarcaster();
            if (!farcaster)
                continue;

            // ensure that the farcaster is connected:
            farcaster->ExecFrame(0);

            // now find the destination:
            const Ship* dest = farcaster->GetDest();
            SimRegion* dest_rgn = (dest && dest->GetRegion()) ? dest->GetRegion() : nullptr;

            if (dest_rgn) {
                FarcastMenu->AddItem(dest_rgn->GetName(), FARCAST);
            }
        }
    }

    // ----------------------------------------------------------
    // BUILD MAIN MENU
    // ----------------------------------------------------------
    MainMenu->AddMenu(Game::GetText("TacView.item.camera"), ViewMenu);
    MainMenu->AddItem("", 0);

    MainMenu->AddItem(Game::GetText("TacView.item.instructions"), VIEW_INS);
    MainMenu->AddItem(Game::GetText("TacView.item.navigation"), VIEW_NAV);

    const ShipDesign* design = ship->Design();
    if (design) {
        if (design->repair_screen)
            MainMenu->AddItem(Game::GetText("TacView.item.engineering"), VIEW_ENG);

        if (design->wep_screen)
            MainMenu->AddItem(Game::GetText("TacView.item.weapons"), VIEW_WEP);
    }

    if (ship->NumFlightDecks() > 0)
        MainMenu->AddItem(Game::GetText("TacView.item.flight"), VIEW_FLT);

    // ----------------------------------------------------------
    // SENSOR / EMCON MENU
    // ----------------------------------------------------------
    EmconMenu->AddItem(Game::GetText("TacView.item.emcon-1"), RadioMessage::GO_EMCON1);
    EmconMenu->AddItem(Game::GetText("TacView.item.emcon-2"), RadioMessage::GO_EMCON2);
    EmconMenu->AddItem(Game::GetText("TacView.item.emcon-3"), RadioMessage::GO_EMCON3);

    if (ship->GetProbeLauncher())
        EmconMenu->AddItem(Game::GetText("TacView.item.probe"), RadioMessage::LAUNCH_PROBE);

    MainMenu->AddItem("", 0);
    MainMenu->AddMenu(Game::GetText("TacView.item.sensors"), EmconMenu);

    // ----------------------------------------------------------
    // QUANTUM MENU (only if quantum drive exists)
    // ----------------------------------------------------------
    QuantumDrive* qdrive = ship->GetQuantumDrive();
    if (qdrive) {
        MainMenu->AddItem("", 0);
        MainMenu->AddMenu(Game::GetText("TacView.item.quantum"), QuantumMenu);
    }

    // ----------------------------------------------------------
    // COMMAND MENU (starships only)
    // ----------------------------------------------------------
    if (ship->IsStarship()) {
        MainMenu->AddItem("", 0);
        MainMenu->AddItem(Game::GetText("TacView.item.command"), VIEW_CMD);
    }
}

// +--------------------------------------------------------------------+

void TacticalView::DrawMenu()
{
    active_menu = 0;

    if (ship)
        active_menu = MainMenu;

    if (msg_ship) {
        if (msg_ship->IsStarship())
            active_menu = StarshipMenu;
        else if (msg_ship->IsDropship())
            active_menu = FighterMenu;
    }

    if (menu_view) {
        menu_view->SetBackColor(hud_color);
        menu_view->SetTextColor(txt_color);
        menu_view->SetMenu(active_menu);
        menu_view->Refresh();
    }
}

// +--------------------------------------------------------------------+

bool TacticalView::GetMouseLoc3D()
{
    int mx = Mouse::X();
    int my = Mouse::Y();

    if (projector) {
        double focal_dist = width / tan(projector->XAngle());

        FVector focal_vect =
            projector->vpn() * focal_dist +
            projector->vup() * -1 * (my - height / 2) +
            projector->vrt() * (mx - width / 2);

        focal_vect.Normalize();

        if (Keyboard::KeyDown(VK_SHIFT)) {
            if (Mouse::RButton())
                return true;

            if (fabs(focal_vect.X) > fabs(focal_vect.Z)) {
                double dx = move_loc.X - projector->Pos().X;
                double t = -1 * ((projector->Pos().X - dx) / focal_vect.X);

                if (t > 0) {
                    FVector p = projector->Pos() + focal_vect * t;
                    move_alt = p.Y - base_alt;
                }
            }
            else {
                double dz = move_loc.Z - projector->Pos().Z;
                double t = -1 * ((projector->Pos().Z - dz) / focal_vect.Z);

                if (t > 0) {
                    FVector p = projector->Pos() + focal_vect * t;
                    move_alt = p.Y - base_alt;
                }
            }

            if (move_alt > 25e3)
                move_alt = 25e3;
            else if (move_alt < -25e3)
                move_alt = -25e3;

            return true;
        }
        else {
            if (fabs(focal_vect.Y) > 1e-5) {
                if (Mouse::RButton())
                    return true;

                bool clamp = false;
                double t = -1 * ((projector->Pos().Y - base_alt) / focal_vect.Y);

                while (t <= 0 && my < height - 1) {
                    my++;
                    clamp = true;

                    focal_vect =
                        projector->vpn() * focal_dist +
                        projector->vup() * -1 * (my - height / 2) +
                        projector->vrt() * (mx - width / 2);

                    focal_vect.Normalize();
                    t = -1 * ((projector->Pos().Y - base_alt) / focal_vect.Y);
                }

                if (t > 0) {
                    if (clamp)
                        Mouse::SetCursorPos(mx, my);

                    move_loc = projector->Pos() + focal_vect * t;
                }

                return true;
            }
        }
    }

    return false;
}

void TacticalView::DrawMove()
{
    if (!projector || !show_move || !msg_ship)
        return;

    FVector origin = msg_ship->Location(); // was: Point

    if (GetMouseLoc3D()) {
        FVector dest = move_loc;

        double distance = (dest - origin).Size();

        projector->Transform(origin);
        projector->Project(origin);

        int x0 = (int)origin.X;
        int y0 = (int)origin.Y;

        projector->Transform(dest);
        projector->Project(dest);

        int x = (int)dest.X;
        int y = (int)dest.Y;

        window->DrawEllipse(x - 10, y - 10, x + 10, y + 10, FColor::White);
        window->DrawLine(x0, y0, x, y, FColor::White);

        char range[32];
        Rect range_rect(x + 12, y - 8, 120, 20);

        if (fabs(move_alt) > 1) {
            dest = move_loc;
            dest.Y += move_alt;
            distance = (dest - msg_ship->Location()).Size();

            projector->Transform(dest);
            projector->Project(dest);

            int x1 = (int)dest.X;
            int y1 = (int)dest.Y;

            window->DrawEllipse(x1 - 10, y1 - 10, x1 + 10, y1 + 10, FColor::White);
            window->DrawLine(x0, y0, x1, y1, FColor::White);
            window->DrawLine(x1, y1, x, y, FColor::White);

            range_rect.x = x1 + 12;
            range_rect.y = y1 - 8;
        }

        FormatNumber(range, distance);
        font->SetColor(FColor::White);
        font->DrawText(range, 0, range_rect, DT_LEFT | DT_SINGLELINE);
        font->SetColor(txt_color);
    }
}

void TacticalView::SendMove()
{
    if (!projector || !show_move || !msg_ship)
        return;

    if (GetMouseLoc3D()) {
        SimElement* elem = msg_ship->GetElement();
        RadioMessage* msg = new RadioMessage(elem, ship, RadioMessage::MOVE_PATROL);

        FVector dest = move_loc;
        dest.Y += move_alt;

        // NOTE: This assumes RadioMessage::SetLocation accepts FVector in your UE port.
        // If SetLocation is still Point-based, update that API to accept FVector.
        msg->SetLocation(dest);

        RadioTraffic::Transmit(msg);
        HUDSounds::PlaySound(HUDSounds::SND_TAC_ACCEPT);
    }
}

// +--------------------------------------------------------------------+

static int invalid_action = false;

void TacticalView::DrawAction()
{
    if (!projector || !show_action || !msg_ship)
        return;

    FVector origin = msg_ship->Location(); // was: Point
    projector->Transform(origin);
    projector->Project(origin);

    int x0 = (int)origin.X;
    int y0 = (int)origin.Y;

    int mx = mouse_action.X;
    int my = mouse_action.Y;
    int r = 10;

    int enemy = 2;
    if (ship->GetIFF() > 1)
        enemy = 1;

    Ship* tgt = WillSelectAt(mx, my);
    int tgt_iff = 0;

    if (tgt)
        tgt_iff = tgt->GetIFF();

    FColor c = FColor::White;

    switch (show_action) {
    case RadioMessage::ATTACK:
    case RadioMessage::BRACKET:
        c = Ship::IFFColor(enemy);
        if (tgt) {
            if (tgt_iff == ship->GetIFF() || tgt_iff == 0)
                r = 0;
        }
        break;

    case RadioMessage::ESCORT:
    case RadioMessage::DOCK_WITH:
        c = ship->MarkerColor();
        if (tgt) {
            if (tgt_iff == enemy)
                r = 0;

            // must have a hangar to dock with...
            if (show_action == RadioMessage::DOCK_WITH && tgt->GetHangar() == 0)
                r = 0;
        }
        break;

    default:
        if (tgt) {
            if (tgt_iff == ship->GetIFF())
                r = 0;
        }
        break;
    }

    if (tgt && r) {
        if ((Game::RealTime() / 200) & 1)
            r = 20;
        else
            r = 15;
    }

    if (r) {
        invalid_action = false;
        window->DrawEllipse(mx - r, my - r, mx + r, my + r, c);
    }
    else {
        invalid_action = true;
        window->DrawLine(mx - 10, my - 10, mx + 10, my + 10, c);
        window->DrawLine(mx + 10, my - 10, mx - 10, my + 10, c);
    }

    window->DrawLine(x0, y0, mx, my, c);
}

void TacticalView::SendAction()
{
    if (!show_action || !msg_ship || invalid_action) {
        HUDSounds::PlaySound(HUDSounds::SND_TAC_REJECT);
        return;
    }

    int mx = mouse_action.X;
    int my = mouse_action.Y;

    Ship* tgt = WillSelectAt(mx, my);

    if (tgt) {
        SimElement* elem = msg_ship->GetElement();
        RadioMessage* msg = new RadioMessage(elem, ship, show_action);

        msg->AddTarget(tgt);

        RadioTraffic::Transmit(msg);
        HUDSounds::PlaySound(HUDSounds::SND_TAC_ACCEPT);
    }
    else {
        HUDSounds::PlaySound(HUDSounds::SND_TAC_REJECT);
    }
}
