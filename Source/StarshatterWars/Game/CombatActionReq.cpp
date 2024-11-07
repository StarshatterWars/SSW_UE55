/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         CombatActionReq.cpp
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	A planned action (mission/story/strategy) in a dynamic campaign.
*/


#include "CombatActionReq.h"


// +----------------------------------------------------------------------+

int
CombatActionReq::CompFromName(const char* n)
{
	int comp = 0;

	if (!_stricmp(n, "LT"))
		comp = COMPARISON_OPERATOR::LT;

	else if (!_stricmp(n, "LE"))
		comp = COMPARISON_OPERATOR::LE;

	else if (!_stricmp(n, "GT"))
		comp = COMPARISON_OPERATOR::GT;

	else if (!_stricmp(n, "GE"))
		comp = COMPARISON_OPERATOR::GE;

	else if (!_stricmp(n, "EQ"))
		comp = COMPARISON_OPERATOR::EQ;

	else if (!_stricmp(n, "RLT"))
		comp = COMPARISON_OPERATOR::RLT;

	else if (!_stricmp(n, "RLE"))
		comp = COMPARISON_OPERATOR::RLE;

	else if (!_stricmp(n, "RGT"))
		comp = COMPARISON_OPERATOR::RGT;

	else if (!_stricmp(n, "RGE"))
		comp = COMPARISON_OPERATOR::RGE;

	else if (!_stricmp(n, "REQ"))
		comp = COMPARISON_OPERATOR::REQ;

	return comp;
}
