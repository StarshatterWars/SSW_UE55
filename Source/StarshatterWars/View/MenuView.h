/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    StarshatterWars
    FILE:         MenuView.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    View class for displaying right-click context menus
*/

#pragma once

// ---------------------------------------------------------------------
// Minimal includes (keep header light)
// ---------------------------------------------------------------------
#include "Types.h"
#include "View.h"
#include "Text.h"

// Minimal UE types used in signatures / members:
#include "Math/Vector.h"               // FVector
#include "Math/Color.h"                // FColor
#include "Math/UnrealMathUtility.h"    // Math

#include "GameStructs.h"               // (must be last include)

// +--------------------------------------------------------------------+
// Forward declarations
// +--------------------------------------------------------------------+

class Menu;
class MenuItem;

// +--------------------------------------------------------------------+

class MenuView : public View
{
public:
    MenuView(View* InParent, int ax = 0, int ay = 0, int aw = 0, int ah = 0);
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
    virtual Menu* GetMenu() { return menu; }
    virtual void      SetMenu(Menu* m) { menu = m; }
    virtual MenuItem* GetMenuItem() { return menu_item; } 

protected:
    int         width = 0;
    int         height = 0;

    int         shift_down = 0;
    int         mouse_down = 0;
    int         right_down = 0;
    int         show_menu = 0;

    // Legacy POINT -> FVector (Z unused, keep at 0)
    FVector     right_start = FVector::ZeroVector;
    FVector     offset = FVector::ZeroVector;

    int         action = 0;
    Menu* menu = nullptr;
    MenuItem* menu_item = nullptr;
    MenuItem* selected = nullptr;
};
