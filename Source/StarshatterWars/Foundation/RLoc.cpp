/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Foundation
	FILE:         RLoc.cpp
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Navigation Point class implementation
*/


#include "RLoc.h"
#include "Random.h"

// +----------------------------------------------------------------------+

RLoc::RLoc()
	: rloc(0), dex(0), dex_var(5.0e3f), az(0), az_var(3.1415f), el(0), el_var(0.1f)
{ }

RLoc::RLoc(const Point& l, double d, double dv)
	: loc(l), base_loc(l), rloc(0), dex((float)d), dex_var((float)dv),
	az(0), az_var(3.1415f), el(0), el_var(0.1f)
{ }

RLoc::RLoc(RLoc* l, double d, double dv)
	: rloc(l), dex((float)d), dex_var((float)dv),
	az(0), az_var(3.1415f), el(0), el_var(0.1f)
{ }

RLoc::RLoc(const RLoc& r)
	: loc(r.loc), base_loc(r.base_loc), rloc(r.rloc),
	dex(r.dex), dex_var(r.dex_var),
	az(r.az), az_var(r.az_var),
	el(r.el), el_var(r.el_var)
{ }

RLoc::~RLoc()
{
}

// +----------------------------------------------------------------------+

const Point&
RLoc::Location()
{
	if (rloc || dex > 0) Resolve();
	return loc;
}

// +----------------------------------------------------------------------+

void
RLoc::Resolve()
{
	if (rloc) {
		base_loc = rloc->Location();
		rloc = 0;
	}

	if (dex > 0) {
		double d = dex + RandomDouble(-dex_var, dex_var);
		double a = az + RandomDouble(-az_var, az_var);
		double e = el + RandomDouble(-el_var, el_var);

		Point  p = Point(d * sin(a),
			d * -cos(a),
			d * sin(e));

		loc = base_loc + p;
		dex = 0;
	}
	else {
		loc = base_loc;
	}
}

// +----------------------------------------------------------------------+

void
RLoc::SetBaseLocation(const Point& l)
{
	base_loc = l;
	loc = l;
}

