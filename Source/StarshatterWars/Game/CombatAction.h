/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         CombatAction.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	A planned action (mission/story/strategy) in a dynamic campaign.
*/

#pragma once

#include "CoreMinimal.h"
#include "../Foundation/Types.h"
#include "../Foundation/Geometry.h"
#include "../Foundation/Text.h"
#include "../Foundation/List.h"


// +--------------------------------------------------------------------+

class Combatant;
class CombatAction;
class CombatActionReq;

// +--------------------------------------------------------------------+

/**
 * 
 */
class STARSHATTERWARS_API CombatAction
{
public:
	static const char* TYPENAME() { return "CombatAction"; }

	CombatAction();
	~CombatAction();

	CombatAction(int id, int type, int subtype, int team);

	int operator == (const CombatAction& a)  const { return id == a.id; }

	bool                 IsAvailable()  const;
	void                 FireAction();
	void                 FailAction();
	void                 AddRequirement(int action, int stat, bool notreq = false);
	void                 AddRequirement(Combatant* c1, Combatant* c2, int comp, int score);
	void                 AddRequirement(Combatant* c1, int group_type, int group_id, int comp, int score, int intel = 0);
	static int           TypeFromName(const char* n);
	static int           StatusFromName(const char* n);

	// accessors/mutators:
	int                  Identity()     const { return id; }
	int                  Type()         const { return type; }
	int                  Subtype()      const { return subtype; }
	int                  OpposingType() const { return opp_type; }
	int                  GetIFF()       const { return team; }
	int                  Status()       const { return status; }
	int                  Source()       const { return source; }
	Point                Location()     const { return loc; }
	const char* System()       const { return system; }
	const char* Region()       const { return region; }
	const char* Filename()     const { return text_file; }
	const char* ImageFile()    const { return image_file; }
	const char* SceneFile()    const { return scene_file; }
	int                  Count()        const { return count; }
	int                  ExecTime()     const { return time; }
	int                  StartBefore()  const { return start_before; }
	int                  StartAfter()   const { return start_after; }
	int                  MinRank()      const { return min_rank; }
	int                  MaxRank()      const { return max_rank; }
	int                  Delay()        const { return delay; }
	int                  Probability()  const { return probability; }
	int                  AssetType()    const { return asset_type; }
	int                  AssetId()      const { return asset_id; }
	List<Text>& AssetKills() { return asset_kills; }
	int                  TargetType()   const { return target_type; }
	int                  TargetId()     const { return target_id; }
	int                  TargetIFF()    const { return target_iff; }
	List<Text>& TargetKills() { return target_kills; }
	const char* GetText()      const { return text; }

	void                 SetType(int t) { type = (char)t; }
	void                 SetSubtype(int s) { subtype = (char)s; }
	void                 SetOpposingType(int t) { opp_type = (char)t; }
	void                 SetIFF(int t) { team = (char)t; }
	void                 SetStatus(int s) { status = (char)s; }
	void                 SetSource(int s) { source = s; }
	void                 SetLocation(const Point& p) { loc = p; }
	void                 SetSystem(Text sys) { system = sys; }
	void                 SetRegion(Text rgn) { region = rgn; }
	void                 SetFilename(Text f) { text_file = f; }
	void                 SetImageFile(Text f) { image_file = f; }
	void                 SetSceneFile(Text f) { scene_file = f; }
	void                 SetCount(int n) { count = (char)n; }
	void                 SetExecTime(int t) { time = t; }
	void                 SetStartBefore(int s) { start_before = s; }
	void                 SetStartAfter(int s) { start_after = s; }
	void                 SetMinRank(int n) { min_rank = (char)n; }
	void                 SetMaxRank(int n) { max_rank = (char)n; }
	void                 SetDelay(int d) { delay = d; }
	void                 SetProbability(int n) { probability = n; }
	void                 SetAssetType(int t) { asset_type = t; }
	void                 SetAssetId(int n) { asset_id = n; }
	void                 SetTargetType(int t) { target_type = t; }
	void                 SetTargetId(int n) { target_id = n; }
	void                 SetTargetIFF(int n) { target_iff = n; }
	void                 SetText(Text t) { text = t; }


private:
	int                  id;
	char                 type;
	char                 subtype;
	char                 opp_type;
	char                 team;
	char                 status;
	char                 min_rank;
	char                 max_rank;
	int                  source;
	Point                loc;
	Text                 system;
	Text                 region;
	Text                 text_file;
	Text                 image_file;
	Text                 scene_file;
	char                 count;
	int                  start_before;
	int                  start_after;
	int                  delay;
	int                  probability;
	int                  rval;
	int                  time;

	Text                 text;
	int                  asset_type;
	int                  asset_id;
	List<Text>           asset_kills;
	int                  target_type;
	int                  target_id;
	int                  target_iff;
	List<Text>           target_kills;

	List<CombatActionReq> requirements;
};
