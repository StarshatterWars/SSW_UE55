/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         PlayerData.cpp
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Player / Logbook class
*/


#include "PlayerData.h"
//#include "NetLobbyServer.h"
//#include "NetLayer.h"
#include "Ship.h"
//#include "SimEvent.h"
#include "Campaign.h"
//#include "CampaignSaveGame.h"
#include "../Foundation/Random.h"
//#include "HUDView.h"
//#include "MFD.h"

#include "../Foundation/DataLoader.h"
//#include "Encrypt.h"
#include "../Foundation/ParseUtil.h"
#include "../Foundation/FormatUtil.h"
//#include "Bitmap.h"
#include "../System/Game.h"

PlayerData::PlayerData()
{
}

PlayerData::~PlayerData()
{
}

int PlayerData::Rank() const
{
	return 0;
}

PlayerData* PlayerData::GetCurrentPlayer()
{
	return nullptr;
}
