// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"

#include "../Foundation/Types.h"
#include "../Foundation/Geometry.h"
#include "SimObject.h"
#include "../Foundation/List.h"
#include "../Foundation/Text.h"

// +--------------------------------------------------------------------+

class Element;
class RadioMessage;
class UShip;
class USimObject;

// +--------------------------------------------------------------------+
/**
 * 
 */
class STARSHATTERWARS_API RadioTraffic
{
public:
	RadioTraffic();
	~RadioTraffic();

	// accessors:
	static void          Initialize();
	static void          Close();

	static RadioTraffic* GetInstance() { return radio_traffic; }

	static void          SendQuickMessage(UShip* ship, int msg);
	static void          Transmit(RadioMessage* msg);
	static void          DiscardMessages();
	static Text          TranslateVox(const char* phrase);

	void                 SendMessage(RadioMessage* msg);
	void                 DisplayMessage(RadioMessage* msg);


protected:
	List<RadioMessage>   traffic;

	static RadioTraffic* radio_traffic;
};
