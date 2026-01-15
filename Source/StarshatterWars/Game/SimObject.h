/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         SimObject.h
	AUTHOR:       Carlos Bott


	OVERVIEW
	========
	Simulation Object and Observer classes
*/

#pragma once

#include "CoreMinimal.h"
#include "Types.h"
#include "List.h"
#include "Physical.h"
#include "SimObject.generated.h"

// +-------------------------------------------------------------------- +

class Sim;
class SimRegion;
class USimObject;
class SimObserver;
class Scene;

// +--------------------------------------------------------------------+

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API USimObject : public UPhysical
{
	GENERATED_BODY()
	
	friend class SimRegion;

public:
	static const char* TYPENAME() { return "SimObject"; }

	enum TYPES {
		SIM_SHIP = 100,
		SIM_SHOT,
		SIM_DRONE,
		SIM_EXPLOSION,
		SIM_DEBRIS,
		SIM_ASTEROID
	};

	USimObject();
	USimObject(const char* n, int t = 0);
	
	~USimObject();

	virtual SimRegion* GetRegion()                const { return region; }
	virtual void         SetRegion(SimRegion* rgn) { region = rgn; }

	virtual void         Notify();
	virtual void         Register(SimObserver* obs);
	virtual void         Unregister(SimObserver* obs);

	virtual void         Activate(Scene& scene);
	virtual void         Deactivate(Scene& scene);

	virtual DWORD        GetObjID()                 const { return objid; }
	virtual void         SetObjID(DWORD oid) { objid = oid; }

	virtual bool         IsHostileTo(const USimObject* o)
		const {
		return false;
	}

protected:
	SimRegion* region;
	List<SimObserver>    observers;
	DWORD                objid;
	bool                 active;
	bool                 notifying;	
};

// +--------------------------------------------------------------------+

class SimObserver
{
public:
	static const char* TYPENAME() { return "SimObserver"; }

	virtual ~SimObserver();

	int operator == (const SimObserver& o) const { return this == &o; }

	virtual bool         Update(USimObject* obj);
	virtual const char*  GetObserverName() const;

	virtual void         Observe(USimObject* obj);
	virtual void         Ignore(USimObject* obj);


protected:
	List<USimObject>      observe_list;
};