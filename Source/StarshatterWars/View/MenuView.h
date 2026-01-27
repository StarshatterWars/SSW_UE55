/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright 2025-2026. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         MenuView.h
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR: John DiCamillo
    ORIGINAL STUDIO: Destroyer Studios LLC

    OVERVIEW
    ========
    View class for displaying right-click context menus
*/

#pragma once

#include "Types.h"
#include "View.h"
#include "SystemFont.h"
#include "Text.h"

#include "Math/Color.h"

// Replace Bitmap with Unreal texture pointer (forward declared to keep header light):
class UTexture2D;

// +--------------------------------------------------------------------+

class Menu;
class MenuItem;

// +--------------------------------------------------------------------+

class MenuView : public UView
{
public:
    MenuView(Window* c);
    virtual ~MenuView();

    // Operations:
    virtual void      Refresh();
    virtual void      OnWindowMove();
    virtual void      DoMouseFrame();
    virtual void      DrawMenu();
    virtual void      DrawMenu(int x, int y, Menu* menu);
    virtual int       ProcessMenuItem();
    virtual void      ClearMenuSelection(Menu* menu);

    virtual bool      IsShown() { return show_menu != 0; }
    virtual int       GetAction() { return action; }
    virtual Menu*     GetMenu() { return menu; }
    virtual void      SetMenu(Menu* m) { menu = m; }
    virtual MenuItem* GetMenuItem() { return menu_item; }

    virtual FColor     GetBackColor() { return back_color; }
    virtual void      SetBackColor(FColor c) { back_color = c; }
    virtual FColor     GetTextColor() { return text_color; }
    virtual void      SetTextColor(FColor c) { text_color = c; }

protected:
    int         width, height;

    int         shift_down;
    int         mouse_down;
    int         right_down;
    int         show_menu;
    POINT       right_start;
    POINT       offset;

    int         action;
    Menu* menu;
    MenuItem* menu_item;
    MenuItem* selected;

    // Previously Bitmap* usage (if any) should now be UTexture2D*.
    // Kept as a forward-declared type to avoid heavy includes:
    UTexture2D* menu_tex = nullptr;

    FColor       back_color;
    FColor       text_color;
};

