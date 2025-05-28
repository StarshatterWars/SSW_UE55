// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "GalaxyManager.h"
#include "Kismet/GameplayStatics.h"
#include "../Foundation/PlanetUtils.h"
#include "Engine/TextureRenderTarget2D.h"

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
UTextureRenderTarget2D* UGalaxyManager::GetOrCreateRenderTarget(const FString& PlanetName, int32 Resolution)
{
	if (PlanetRenderTargets.Contains(PlanetName))
	{
		return PlanetRenderTargets[PlanetName];
	}

	UTextureRenderTarget2D* NewRT = PlanetUtils::CreatePlanetRenderTarget(PlanetName, nullptr, Resolution);
	if (NewRT)
	{
		PlanetRenderTargets.Add(PlanetName, NewRT);
	}
	return NewRT;
}

void UGalaxyManager::ClearAllRenderTargets()
{
	for (auto& Pair : PlanetRenderTargets)
	{
		if (Pair.Value)
		{
			// Mark for garbage collection
			Pair.Value->MarkAsGarbage();
		}
	}
	PlanetRenderTargets.Empty();

	UE_LOG(LogTemp, Warning, TEXT("[GalaxyManager] Cleared all planet render targets."));
}

