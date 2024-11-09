/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         ComnbatEvent.cpp
	AUTHOR:       Carlos Bott


	OVERVIEW
	========
	A significant (newsworthy) event in the dynamic campaign.
*/

#pragma once

#include "CoreMinimal.h"
#include "../Foundation/Types.h"
#include "../Foundation/Geometry.h"
//#include "Bitmap.h"
#include "../Foundation/Text.h"
#include "../Foundation/List.h"
#include "../System/SSWGameInstance.h"

// +--------------------------------------------------------------------+

class ACampaign;
class CombatGroup;
class CombatUnit;

// +--------------------------------------------------------------------+

/**
 * 
 */
class STARSHATTERWARS_API CombatEvent
{
public:
	
	static const char* TYPENAME() { return "CombatEvent"; }

	enum EVENT_TYPE {
		ATTACK,
		DEFEND,
		MOVE_TO,
		CAPTURE,
		STRATEGY,

		CAMPAIGN_START,
		STORY,
		CAMPAIGN_END,
		CAMPAIGN_FAIL
	};

	enum EVENT_SOURCE {
		FORCOM,
		TACNET,
		INTEL,
		MAIL,
		NEWS
	};

	CombatEvent();
	~CombatEvent();

	CombatEvent(ACampaign* c, int type, int time, int team, int source, const char* rgn);

	int operator == (const CombatEvent& u)  const { return this == &u; }

	// accessors/mutators:
	int                  Type()         const { return type; }
	int                  Time()         const { return time; }
	int                  GetIFF()       const { return team; }
	int                  Points()       const { return points; }
	int                  Source()       const { return source; }
	Point                Location()     const { return loc; }
	const char* Region()       const { return region; }
	const char* Title()        const { return title; }
	const char* Information()  const { return info; }
	const char* Filename()     const { return file; }
	const char* ImageFile()    const { return image_file; }
	const char* SceneFile()    const { return scene_file; }
	//Bitmap& Image() { return image; }
	const char* SourceName()   const;
	const char* TypeName()     const;
	bool                 Visited()      const { return visited; }

	void                 SetType(int t) { type = t; }
	void                 SetTime(int t) { time = t; }
	void                 SetIFF(int t) { team = t; }
	void                 SetPoints(int p) { points = p; }
	void                 SetSource(int s) { source = s; }
	void                 SetLocation(const Point& p) { loc = p; }
	void                 SetRegion(Text rgn) { region = rgn; }
	void                 SetTitle(Text t) { title = t; }
	void                 SetInformation(Text t) { info = t; }
	void                 SetFilename(Text f) { file = f; }
	void                 SetImageFile(Text f) { image_file = f; }
	void                 SetSceneFile(Text f) { scene_file = f; }
	void                 SetVisited(bool v) { visited = v; }

	// operations:
	void                 Load();

	// utilities:
	static int           TypeFromName(const char* n);
	static int           SourceFromName(const char* n);
	static const char* TypeName(int n);
	static const char* SourceName(int n);

private:
	ACampaign* campaign;
	int                  type;
	int                  time;
	int                  team;
	int                  points;
	int                  source;
	bool                 visited;
	Point                loc;
	Text                 region;
	Text                 title;
	Text                 info;
	Text                 file;
	Text                 image_file;
	Text                 scene_file;
	//Bitmap               image;
};
