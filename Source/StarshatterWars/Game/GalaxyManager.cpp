// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "GalaxyManager.h"
#include "Kismet/GameplayStatics.h"

UGalaxyManager* UGalaxyManager::Get(UObject* WorldContext)
{
	static UGalaxyManager* Singleton = nullptr;
	if (!Singleton)
	{
		Singleton = NewObject<UGalaxyManager>();
		Singleton->AddToRoot(); // avoid GC
	}
	return Singleton;
}

void UGalaxyManager::LoadGalaxy(const TArray<FS_Galaxy>& ParsedSystems)
{
	Systems = ParsedSystems;
}

const FS_Galaxy* UGalaxyManager::FindSystemByName(const FString& Name) const
{
	for (const FS_Galaxy& G : Systems)
	{
		if (G.Name == Name)
		{
			return &G;
		}
	}
	return nullptr;
}

