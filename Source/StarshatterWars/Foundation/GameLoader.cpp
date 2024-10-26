/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Space
	FILE:         GameLoader.cpp
	AUTHOR:       Carlos Bott
*/

#include "GameLoader.h"

void AGameLoader::BeginPlay()
{
	LoadGalaxy();
}

void AGameLoader::Tick(float DeltaTime)
{
}

USSWGameInstance* AGameLoader::GetSSWGameInstance()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	return SSWInstance;
}

void AGameLoader::LoadUniverse()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->SpawnUniverse();
}

void AGameLoader::LoadGalaxy()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->SpawnGalaxy();
}

