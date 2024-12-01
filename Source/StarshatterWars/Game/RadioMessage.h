// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "../Foundation/Types.h"
#include "../Foundation/Geometry.h"
#include "SimObject.h"
#include "Instruction.h"
#include "../Foundation/List.h"
#include "../Foundation/Text.h"

// +--------------------------------------------------------------------+

class Element;
class UShip;
class USimObject;

// +--------------------------------------------------------------------+
/**
 * 
 */
class STARSHATTERWARS_API RadioMessage
{
public:
	
	enum  ACTION
	{
		NONE = 0,

		DOCK_WITH = Instruction::DOCK,
		RTB = Instruction::RTB,
		QUANTUM_TO = Instruction::NUM_ACTIONS,
		FARCAST_TO,

		// protocol:
		ACK,
		NACK,

		// target mgt:
		ATTACK,
		ESCORT,
		BRACKET,
		IDENTIFY,

		// combat mgt:
		COVER_ME,
		WEP_FREE,
		WEP_HOLD,
		FORM_UP,       // alias for wep_hold
		SAY_POSITION,

		// sensor mgt:
		LAUNCH_PROBE,
		GO_EMCON1,
		GO_EMCON2,
		GO_EMCON3,

		// formation mgt:
		GO_DIAMOND,
		GO_SPREAD,
		GO_BOX,
		GO_TRAIL,

		// mission mgt:
		MOVE_PATROL,
		SKIP_NAVPOINT,
		RESUME_MISSION,

		// misc announcements:
		CALL_ENGAGING,
		FOX_1,
		FOX_2,
		FOX_3,
		SPLASH_1,
		SPLASH_2,
		SPLASH_3,
		SPLASH_4,
		SPLASH_5,   // target destroyed
		SPLASH_6,   // enemy destroyed
		SPLASH_7,   // confirmed kill
		DISTRESS,
		BREAK_ORBIT,
		MAKE_ORBIT,
		QUANTUM_JUMP,

		// friendly fire:
		WARN_ACCIDENT,
		WARN_TARGETED,
		DECLARE_ROGUE,

		// support:
		PICTURE,
		REQUEST_PICTURE,
		REQUEST_SUPPORT,

		// traffic control:
		CALL_INBOUND,
		CALL_APPROACH,
		CALL_CLEARANCE,
		CALL_FINALS,
		CALL_WAVE_OFF,

		NUM_ACTIONS
	};

	RadioMessage();
	~RadioMessage();

	RadioMessage(UShip* dst, const UShip* sender, int action);
	RadioMessage(Element* dst, const UShip* sender, int action);
	RadioMessage(const RadioMessage& rm);

	// accessors:
	static const char* ActionName(int a);

	const UShip* Sender()          const { return sender; }
	UShip* DestinationShip() const { return dst_ship; }
	Element* DestinationElem() const { return dst_elem; }
	int               Action()          const { return action; }
	List<USimObject>& TargetList() { return target_list; }
	const Point& Location()        const { return location; }
	const Text& Info()            const { return info; }
	int               Channel()         const { return channel; }

	// mutators:
	void              SetDestinationShip(UShip* s) { dst_ship = s; }
	void              SetDestinationElem(Element* e) { dst_elem = e; }
	void              AddTarget(USimObject* s);
	void              SetLocation(const Point& l) { location = l; }
	void              SetInfo(Text msg) { info = msg; }
	void              SetChannel(int c) { channel = c; }

protected:
	const UShip*	  sender;
	UShip*			  dst_ship;
	Element*		  dst_elem;
	int               action;
	List<USimObject>  target_list;
	Point             location;
	Text              info;
	int               channel;
};
