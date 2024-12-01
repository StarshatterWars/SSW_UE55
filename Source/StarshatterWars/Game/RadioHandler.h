// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
#include "../Foundation/Types.h"
#include "../Foundation/Geometry.h"
#include "SimObject.h"

 // +--------------------------------------------------------------------+

class RadioMessage;
class UShip;

// +-----------------------

class STARSHATTERWARS_API RadioHandler
{
public:
	RadioHandler();
	~RadioHandler();

	virtual bool      ProcessMessage(RadioMessage* msg, UShip* s);
	virtual void      AcknowledgeMessage(RadioMessage* msg, UShip* s);

protected:
	virtual bool      IsOrder(int action);
	virtual bool      ProcessMessageOrders(RadioMessage* msg, UShip* s);
	virtual bool      ProcessMessageAction(RadioMessage* msg, UShip* s);

	virtual bool      Inbound(RadioMessage* msg, UShip* s);
	virtual bool      Picture(RadioMessage* msg, UShip* s);
	virtual bool      Support(RadioMessage* msg, UShip* s);
	virtual bool      SkipNavpoint(RadioMessage* msg, UShip* s);
	virtual bool      LaunchProbe(RadioMessage* msg, UShip* s);
};
