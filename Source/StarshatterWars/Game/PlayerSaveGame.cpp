// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "PlayerSaveGame.h"
#include "GameStructs.h"

UPlayerSaveGame::UPlayerSaveGame()
{
	PlayerInfo.Id = 0;
	PlayerInfo.Name = "Default Player";
	PlayerInfo.Nickname = "";
	PlayerInfo.Campaign = 0;
	PlayerInfo.Avatar = 0;
	PlayerInfo.Mission = 0;
	PlayerInfo.Rank = 0;
	PlayerInfo.Empire = 0;
	PlayerInfo.ShipColor = 0;
	PlayerInfo.HudMode = 0;
	PlayerInfo.GunMode = 0;
	PlayerInfo.HudColor = 0;
	PlayerInfo.FlightModel = 0;
	PlayerInfo.LandingMode = 0;
	PlayerInfo.FlyingStart = false;
	PlayerInfo.GridMode = false;
	PlayerInfo.TrainingMode = false;
	PlayerInfo.GunSightMode = false;
	PlayerInfo.CreateTime = 0;
	PlayerInfo.FlightTime = 0;
	PlayerInfo.PlayerKills = 0;
	PlayerInfo.PlayerWins = 0;
	PlayerInfo.PlayerLosses = 0;
	PlayerInfo.PlayerPoints = 0;
	PlayerInfo.PlayerLevel = 0;
	PlayerInfo.PlayerExperience = 0;
	PlayerInfo.PlayerStatus = "";
	PlayerInfo.PlayerShip = "";
	PlayerInfo.PlayerSystem = "";
	PlayerInfo.PlayerRegion = "";
	PlayerInfo.PlayerForce = 1;
	PlayerInfo.PlayerFleet = -1;
	PlayerInfo.PlayerWing = -1;
	PlayerInfo.PlayerCarrier = - 1;
	PlayerInfo.PlayerBattleGroup = -1;
	PlayerInfo.PlayerDesronGroup = -1;
	PlayerInfo.PlayerSquadron = -1;
}
