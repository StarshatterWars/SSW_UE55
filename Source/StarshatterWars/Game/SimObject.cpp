/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         SimObject.cpp
	AUTHOR:       Carlos Bott


	OVERVIEW
	========
	Simulation Object and Observer classes
*/

#include "SimObject.h"

//#include "Graphic.h"
//#include "Light.h"
//#include "Scene.h"

SimObserver::~SimObserver()
{
	ListIter<USimObject> observed = observe_list;
	while (++observed)
		observed->Unregister(this);
}

void
SimObserver::Observe(USimObject* obj)
{
	if (obj) {
		obj->Register(this);

		if (!observe_list.contains(obj))
			observe_list.append(obj);
	}
}

void
SimObserver::Ignore(USimObject* obj)
{
	if (obj) {
		obj->Unregister(this);
		observe_list.remove(obj);
	}
}

bool
SimObserver::Update(USimObject* obj)
{
	if (obj)
		observe_list.remove(obj);

	return true;
}

const char*
SimObserver::GetObserverName() const
{
	static char name[32];
	//sprintf_s(name, "SimObserver 0x%08x", (DWORD)this);
	return name;
}

// +--------------------------------------------------------------------+

USimObject::USimObject()
{
	region = 0;
	objid = 0;
	active = 0;
	notifying = 0;
}

USimObject::USimObject(const char* n, int t)
{
	UPhysical(n, t);
	region = 0;
	objid = 0;
	active = 0;
	notifying = 0;
}

USimObject::~USimObject()
{
	Notify();
}

// +--------------------------------------------------------------------+

void
USimObject::Notify()
{
	if (!notifying) {
		notifying = true;

		int nobservers = observers.size();
		int nupdate = 0;

		if (nobservers > 0) {
			ListIter<SimObserver> iter = observers;
			while (++iter) {
				SimObserver* observer = iter.value();
				observer->Update(this);
				nupdate++;
			}

			observers.clear();
		}

		if (nobservers != nupdate) {
			::Print("WARNING: incomplete notify sim object '%s' - %d of %d notified\n",
				Name(), nupdate, nobservers);
		}

		notifying = false;
	}
	else {
		::Print("WARNING: double notify on sim object '%s'\n", Name());
	}
}

// +--------------------------------------------------------------------+

void
USimObject::Register(SimObserver* observer)
{
	if (!notifying && !observers.contains(observer))
		observers.append(observer);
}

// +--------------------------------------------------------------------+

void
USimObject::Unregister(SimObserver* observer)
{
	if (!notifying)
		observers.remove(observer);
}

// +--------------------------------------------------------------------+

void
USimObject::Activate(Scene& scene)
{
	//if (rep)
	//	scene.AddGraphic(rep);
	//if (light)
	//	scene.AddLight(light);

	active = true;
}

void
USimObject::Deactivate(Scene& scene)
{
	//if (rep)
	//	scene.DelGraphic(rep);
	//if (light)
	//	scene.DelLight(light);

	active = false;
}



