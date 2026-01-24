// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "Menu.h"


// +--------------------------------------------------------------------+

// Core menus
static Menu* MainMenu = nullptr;
static Menu* ViewMenu = nullptr;
static Menu* EmconMenu = nullptr;

// Ship / combat menus
static Menu* FighterMenu = nullptr;
static Menu* StarshipMenu = nullptr;
static Menu* TargetMenu = nullptr;
static Menu* CombatMenu = nullptr;
static Menu* FormationMenu = nullptr;
static Menu* SensorsMenu = nullptr;
static Menu* QuantumMenu = nullptr;

// Mission / control menus
static Menu* MissionMenu = nullptr;
static Menu* WingMenu = nullptr;
static Menu* ElementMenu = nullptr;
static Menu* ControlMenu = nullptr;

// +--------------------------------------------------------------------+

void
Menu::AddItem(Text label, uintptr_t value, bool enabled)
{
	MenuItem* item = new MenuItem(label, value, enabled);

	if (item) {
		item->menu = this;
		items.append(item);
	}
}

void
Menu::AddItem(MenuItem* item)
{
	if (item->submenu)
		item->submenu->SetParent(this);
	item->menu = this;
	items.append(item);
}

void
Menu::AddMenu(Text label, Menu* menu, DWORD value)
{
	MenuItem* item = new MenuItem(label, value);

	if (item) {
		item->menu = this;
		item->submenu = menu;
		menu->parent = this;

		items.append(item);
	}
}

MenuItem*
Menu::GetItem(int index)
{
	if (index >= 0 && index < items.size())
		return items[index];

	return 0;
}

void
Menu::SetItem(int index, MenuItem* item)
{
	if (item && index >= 0 && index < items.size())
		items[index] = item;
}

int
Menu::NumItems() const
{
	return items.size();
}

void
Menu::ClearItems()
{
	items.destroy();
}


// +--------------------------------------------------------------------+

MenuItem::MenuItem(Text label, DWORD value, bool e)
	: text(label), data(value), enabled(e), submenu(0), selected(0)
{
}

MenuItem::~MenuItem()
{
}

// +--------------------------------------------------------------------+

Menu*
MenuHistory::GetCurrent()
{
	int n = history.size();

	if (n)
		return history[n - 1];

	return 0;
}

Menu*
MenuHistory::GetLevel(int n)
{
	if (n >= 0 && n < history.size())
		return history[n];

	return 0;
}

Menu*
MenuHistory::Find(const char* title)
{
	for (int i = 0; i < history.size(); i++)
		if (history[i]->GetTitle() == title)
			return history[i];

	return 0;
}

void
MenuHistory::Pop()
{
	int n = history.size();

	if (n)
		history.removeIndex(n - 1);
}

void
MenuHistory::Push(Menu* menu)
{
	history.append(menu);
}

void
MenuHistory::Clear()
{
	history.clear();
}

