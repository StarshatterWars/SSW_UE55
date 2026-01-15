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


#include "CombatEvent.h"
#include "CombatGroup.h"
#include "Campaign.h"
//#include "Player.h"
//#include "ShipDesign.h"
//#include "Ship.h"

#include "Term.h"
#include "ParseUtil.h"
#include "FormatUtil.h"
#include "DataLoader.h"

CombatEvent::CombatEvent()
{
}

CombatEvent::~CombatEvent()
{
}

// +----------------------------------------------------------------------+

CombatEvent::CombatEvent(UCampaign* c, int typ, int tim, int tem,
	int src, const char* rgn)
	: campaign(c), type(typ), time(tim), team(tem), source(src),
	region(rgn), points(0), visited(false)
{ }

// +----------------------------------------------------------------------+

const char*
CombatEvent::SourceName() const
{
	return SourceName(source);
}

// +----------------------------------------------------------------------+

const char*
CombatEvent::TypeName() const
{
	return TypeName(type);
}

// +----------------------------------------------------------------------+

const char*
CombatEvent::SourceName(int n)
{
	switch (n) {
	case ECOMBATEVENT_SOURCE::FORCOM:      return "FORCOM";
	case ECOMBATEVENT_SOURCE::TACNET:      return "TACNET";
	case ECOMBATEVENT_SOURCE::INTEL:       return "SECURE";
	case ECOMBATEVENT_SOURCE::MAIL:        return "Mail";
	case ECOMBATEVENT_SOURCE::NEWS:        return "News";
	}

	return "Unknown";
}

int
CombatEvent::SourceFromName(const char* n)
{
	for (int i = FORCOM; i <= NEWS; i++)
		if (!_stricmp(n, SourceName(i)))
			return i;

	return -1;
}

// +----------------------------------------------------------------------+

const char*
CombatEvent::TypeName(int n)
{
	switch (n) {
	case ECOMBATEVENT_TYPE::ATTACK:            return "ATTACK";
	case ECOMBATEVENT_TYPE::DEFEND:            return "DEFEND";
	case ECOMBATEVENT_TYPE::MOVE_TO:           return "MOVE_TO";
	case ECOMBATEVENT_TYPE::CAPTURE:           return "CAPTURE";
	case ECOMBATEVENT_TYPE::STRATEGY:          return "STRATEGY";
	case ECOMBATEVENT_TYPE::STORY:             return "STORY";
	case ECOMBATEVENT_TYPE::CAMPAIGN_START:    return "CAMPAIGN_START";
	case ECOMBATEVENT_TYPE::CAMPAIGN_END:      return "CAMPAIGN_END";
	case ECOMBATEVENT_TYPE::CAMPAIGN_FAIL:     return "CAMPAIGN_FAIL";
	}

	return "Unknown";
}

int
CombatEvent::TypeFromName(const char* n)
{
	for (int i = ATTACK; i <= CAMPAIGN_FAIL; i++)
		if (!_stricmp(n, TypeName(i)))
			return i;

	return -1;
}

// +----------------------------------------------------------------------+

void
CombatEvent::Load()
{
	DataLoader* loader = DataLoader::GetLoader();

	if (!campaign || !loader)
		return;

	loader->SetDataPath(campaign->Path());

	/*if (file.length() > 0) {
		const char* filename = file.data();
		BYTE* block = 0;

		loader->LoadBuffer(filename, block, true);
		info = (const char*)block;
		loader->ReleaseBuffer(block);

		if (info.contains('$')) {
			Player* player = Player::GetCurrentPlayer();
			CombatGroup* group = campaign->GetPlayerGroup();

			if (player) {
				info = FormatTextReplace(info, "$NAME", player->Name().data());
				info = FormatTextReplace(info, "$RANK", Player::RankName(player->Rank()));
			}

			if (group) {
				info = FormatTextReplace(info, "$GROUP", group->GetDescription());
			}

			char timestr[32];
			FormatDayTime(timestr, campaign->GetTime(), true);
			info = FormatTextReplace(info, "$TIME", timestr);
		}
	}

	if (type < CAMPAIGN_END && image_file.length() > 0) {
		loader->LoadBitmap(image_file, image);
	}

	loader->SetDataPath(0);
	*/
}
