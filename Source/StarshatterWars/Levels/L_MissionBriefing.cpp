// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "L_MissionBriefing.h"

void AL_MissionBriefing::BeginPlay()
{
	Super::BeginPlay();
	ShowMissionBrieingScreen();
}

void AL_MissionBriefing::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AL_MissionBriefing::ShowMissionBrieingScreen()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->ShowMissionBriefingScreen();
}
