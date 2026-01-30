/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    StarshatterWars
    FILE:         MenuView.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    View class for displaying right-click context menus
*/

#include "MenuView.h"

#include "Window.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "MouseController.h"
#include "Menu.h"
#include "UIButton.h"
#include "Game.h"

#include "Logging/LogMacros.h"

// +--------------------------------------------------------------------+
// Local helpers
// +--------------------------------------------------------------------+

static FColor ScaleColor(const FColor& C, float S)
{
    const int32 R = FMath::Clamp(FMath::RoundToInt(C.R * S), 0, 255);
    const int32 G = FMath::Clamp(FMath::RoundToInt(C.G * S), 0, 255);
    const int32 B = FMath::Clamp(FMath::RoundToInt(C.B * S), 0, 255);
    return FColor((uint8)R, (uint8)G, (uint8)B, C.A);
}

// +--------------------------------------------------------------------+

MenuView::MenuView(View* InParent)
    : View(InParent,
        0,
        0,
        InParent ? InParent->Width() : 0,
        InParent ? InParent->Height() : 0)
    , shift_down(0)
    , mouse_down(0)
    , right_down(0)
    , show_menu(false)
    , action(0)
    , menu(nullptr)
    , menu_item(nullptr)
    , selected(nullptr)
{
    right_start = FVector::ZeroVector;
    offset = FVector::ZeroVector;

    // Ensure we match parent after construction (in case parent rect changes later)
    OnWindowMove();
}

MenuView::~MenuView()
{
}

// +--------------------------------------------------------------------+

void MenuView::OnWindowMove()
{
    offset.X = (float)window->X();
    offset.Y = (float)window->Y();
    offset.Z = 0.0f;

    width = window->Width();
    height = window->Height();
}

// +--------------------------------------------------------------------+

void MenuView::Refresh()
{
    if (show_menu)
        DrawMenu();
}

// +--------------------------------------------------------------------+

void MenuView::DoMouseFrame()
{
    static uint32 rbutton_latch = 0;

    action = 0;

    if (Mouse::RButton()) {
        MouseController* mouse_con = MouseController::GetInstance();
        if (!right_down && (!mouse_con || !mouse_con->Active())) {
            rbutton_latch = (uint32)Game::RealTime();
            right_down = true;
            show_menu = false;
        }
    }
    else {
        if (right_down && (Game::RealTime() - rbutton_latch < 250)) {
            right_start.X = (float)(Mouse::X() - (int)offset.X);
            right_start.Y = (float)(Mouse::Y() - (int)offset.Y);
            right_start.Z = 0.0f;

            show_menu = true;
            UIButton::PlaySound(UIButton::SND_MENU_OPEN);
        }

        right_down = false;
    }

    MouseController* mouse_con = MouseController::GetInstance();

    if (!mouse_con || !mouse_con->Active()) {
        if (Mouse::LButton()) {
            if (!mouse_down)
                shift_down = Keyboard::KeyDown(VK_SHIFT);

            mouse_down = true;
        }
        else if (mouse_down) {
            const int mouse_x = Mouse::X() - (int)offset.X;
            const int mouse_y = Mouse::Y() - (int)offset.Y;
            int       keep_menu = false;

            if (show_menu) {
                keep_menu = ProcessMenuItem();
                Mouse::Show(true);
            }

            mouse_down = false;

            if (!keep_menu) {
                ClearMenuSelection(menu);
                show_menu = false;
            }
        }
    }
}

// +--------------------------------------------------------------------+

int MenuView::ProcessMenuItem()
{
    if (!menu_item || !menu_item->GetEnabled())
        return false;

    if (menu_item->GetSubmenu()) {
        ListIter<MenuItem> item = menu_item->GetMenu()->GetItems();
        while (++item) {
            if (item.value() == menu_item)
                item->SetSelected(2);
            else
                item->SetSelected(0);
        }

       UIButton::PlaySound(UIButton::SND_MENU_OPEN);
        return true; // keep menu showing
    }

    action = menu_item->GetData();
    UIButton::PlaySound(UIButton::SND_MENU_SELECT);
    return false;
}

// +--------------------------------------------------------------------+

void MenuView::DrawMenu()
{
    menu_item = nullptr;

    if (menu) {
        const int mx = (int)right_start.X - 2;
        const int my = (int)right_start.Y - 2;
        DrawMenu(mx, my, menu);
    }
}

void MenuView::DrawMenu(int mx, int my, Menu* m)
{
    if (!m)
        return;

    MenuItem* locked_item = nullptr;
    Menu* submenu = nullptr;
    int           subx = 0;
    int           suby = 0;

    Rect menu_rect(mx, my, 100, m->NumItems() * 10 + 6);

    int max_width = 0;
    int max_height = 0;
    int extra_width = 16;

    ListIter<MenuItem> item = m->GetItems();
    while (++item) {
        menu_rect.w = width / 2;

        if (item->GetText().length()) {
            window->SetFont(HUDFont);
            window->DrawText(item->GetText(), 0, menu_rect, DT_LEFT | DT_SINGLELINE | DT_CALCRECT);

            if (menu_rect.w > max_width)
                max_width = menu_rect.w;

            max_height += 11;

            if (item->GetSubmenu())
                extra_width = 28;

            if (item->GetSelected() > 1)
                locked_item = item.value();
        }
        else {
            max_height += 4;
        }
    }

    menu_rect.h = max_height + 6;
    menu_rect.w = max_width + extra_width;

    if (menu_rect.x + menu_rect.w >= width)
        menu_rect.x = width - menu_rect.w - 2;

    if (menu_rect.y + menu_rect.h >= height)
        menu_rect.y = height - menu_rect.h - 2;

    window->FillRect(menu_rect, ScaleColor(BackColor, 0.20f));
    window->DrawRect(menu_rect, BackColor);

    Rect item_rect = menu_rect;

    item_rect.x += 4;
    item_rect.y += 3;
    item_rect.w -= 8;
    item_rect.h = 12;

    item.reset();
    while (++item) {
        int line_height = 0;

        if (item->GetText().length()) {
            Rect fill_rect = item_rect;
            fill_rect.Inflate(2, -1);
            fill_rect.y -= 1;

            const int mouse_x = Mouse::X() - (int)offset.X;
            const int mouse_y = Mouse::Y() - (int)offset.Y;

            // is this item picked?
            if (menu_rect.Contains(mouse_x, mouse_y)) {
                if (mouse_y >= fill_rect.y && mouse_y <= fill_rect.y + fill_rect.h) {
                    if (Mouse::LButton()) {
                        menu_item = item.value();
                        item->SetSelected(2);

                        if (locked_item && locked_item->GetMenu() == m)
                            locked_item->SetSelected(0);

                        locked_item = menu_item;
                    }
                    else if (!locked_item || locked_item->GetMenu() != m) {
                        item->SetSelected(true);
                        menu_item = item.value();
                    }

                    if (menu_item && menu_item != selected) {
                        selected = menu_item;
                        UIButton::PlaySound(UIButton::SND_MENU_HILITE);
                    }
                }
                else if (item.value() != locked_item) {
                    item->SetSelected(false);
                }
            }

            if (item->GetSelected()) {
                window->FillRect(fill_rect, ScaleColor(BackColor, 0.35f));
                window->DrawRect(fill_rect, ScaleColor(BackColor, 0.75f));

                if (item->GetSubmenu()) {
                    submenu = item->GetSubmenu();
                    subx = menu_rect.x + max_width + extra_width;
                    suby = fill_rect.y - 3;
                }
            }

            if (item->GetEnabled())
                HUDFont->SetColor(TextColor);
            else
                HUDFont->SetColor(ScaleColor(TextColor, 0.33f));

            window->SetFont(HUDFont);
            window->DrawText(item->GetText(), 0, item_rect, DT_LEFT | DT_SINGLELINE);
            line_height = 11;
        }
        else {
            window->DrawLine(
                item_rect.x,
                item_rect.y + 2,
                item_rect.x + max_width + extra_width - 8,
                item_rect.y + 2,
                BackColor
            );
            line_height = 4;
        }

        if (item->GetSubmenu()) {
            const int left = item_rect.x + max_width + 10;
            const int top = item_rect.y + 1;

            // draw the arrow (Point -> FVector; Z unused):
            FVector arrow[3];
            arrow[0] = FVector((float)left, (float)top, 0.0f);
            arrow[1] = FVector((float)left + 8, (float)top + 4, 0.0f);
            arrow[2] = FVector((float)left, (float)top + 8, 0.0f);

            window->FillPoly(3, arrow, BackColor);
        }

        item_rect.y += line_height;
    }

    if (submenu) {
        if (subx + 60 > width)
            subx = menu_rect.x - 60;

        DrawMenu(subx, suby, submenu);
    }
}

// +--------------------------------------------------------------------+

void MenuView::ClearMenuSelection(Menu* m)
{
    if (!m)
        return;

    ListIter<MenuItem> item = m->GetItems();
    while (++item) {
        item->SetSelected(0);

        if (item->GetSubmenu())
            ClearMenuSelection(item->GetSubmenu());
    }
}
