// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "GalaxyManager.h"
#include "Kismet/GameplayStatics.h"
#include "../Foundation/PlanetUtils.h"
#include "../Foundation/StarUtils.h"
#include "../Foundation/MoonUtils.h"
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

UTextureRenderTarget2D* UGalaxyManager::GetOrCreatePlanetRenderTarget(const FString& PlanetName, int32 Resolution, UObject* Planet /*= nullptr*/)
{
	if (PlanetRenderTargets.Contains(PlanetName))
	{
		return PlanetRenderTargets[PlanetName];
	}

	UTextureRenderTarget2D* NewRT = PlanetUtils::CreatePlanetRenderTarget(PlanetName, Planet, Resolution);
	if (NewRT)
	{
		PlanetRenderTargets.Add(PlanetName, NewRT);
	}
	return NewRT;
}

UTextureRenderTarget2D* UGalaxyManager::GetOrCreateMoonRenderTarget(const FString& MoonName, int32 Resolution, UObject* Moon)
{
	if (MoonRenderTargets.Contains(MoonName))
	{
		return MoonRenderTargets[MoonName];
	}

	UTextureRenderTarget2D* NewRT = MoonUtils::CreateMoonRenderTarget(MoonName, Moon, Resolution);
	if (NewRT)
	{
		PlanetRenderTargets.Add(MoonName, NewRT);
	}
	return NewRT;
}

UTextureRenderTarget2D* UGalaxyManager::GetOrCreateStarRenderTarget(const FString& StarName,int32 Resolution, UObject* Star)
{
	if (StarRenderTargets.Contains(StarName))
	{
		return StarRenderTargets[StarName];
	}

	UTextureRenderTarget2D* NewRT = StarUtils::CreateStarRenderTarget(StarName, Star, Resolution);
	if (NewRT)
	{
		StarRenderTargets.Add(StarName, NewRT);
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

	for (auto& MoonPair : MoonRenderTargets)
	{
		if (MoonPair.Value)
		{
			// Mark for garbage collection
			MoonPair.Value->MarkAsGarbage();
		}
	}
	MoonRenderTargets.Empty();

	for (auto& StarPair : StarRenderTargets)
	{
		if (StarPair.Value)
		{
			// Mark for garbage collection
			StarPair.Value->MarkAsGarbage();
		}
	}
	StarRenderTargets.Empty();

	UE_LOG(LogTemp, Warning, TEXT("[GalaxyManager] Cleared all render targets."));
}

