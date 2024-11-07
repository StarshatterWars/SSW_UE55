/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         TemplateList.cpp
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Campaign defines a strategic military scenario.
*/


#include "TemplateList.h"


// +====================================================================+

TemplateList::TemplateList() {
	mission_type = 0;
	group_type = 0;
	index = 0;

}

TemplateList::~TemplateList()
{
	//missions.destroy();
}

// +====================================================================+