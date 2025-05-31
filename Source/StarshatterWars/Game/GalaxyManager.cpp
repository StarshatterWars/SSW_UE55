// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "GalaxyManager.h"
#include "Kismet/GameplayStatics.h"
#include "../Foundation/PlanetUtils.h"
#include "../Foundation/StarUtils.h"
#include "../Foundation/SystemMapUtils.h"
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

UTextureRenderTarget2D* UGalaxyManager::GetOrCreateRenderTarget(const FString& Name, int32 Resolution, UObject* Object)
{
	if (RenderTargets.Contains(Name))
	{
		return RenderTargets[Name];
	}

	UTextureRenderTarget2D* NewRT = SystemMapUtils::CreateRenderTarget(Name, Resolution, Object);
	if (NewRT)
	{
		RenderTargets.Add(Name, NewRT);
	}
	return NewRT;
}

void UGalaxyManager::ClearAllRenderTargets()
{
	for (auto& Pair : RenderTargets)
	{
		if (Pair.Value)
		{
			// Mark for garbage collection
			Pair.Value->MarkAsGarbage();
		}
	}
	RenderTargets.Empty();

	UE_LOG(LogTemp, Warning, TEXT("[GalaxyManager] Cleared all render targets."));
}

UTextureRenderTarget2D* UGalaxyManager::GetOrCreateSystemOverviewRenderTarget(
	UWorld* World,
	const FBox2D& ContentBounds,
	float Padding)
{
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("SystemOverviewRenderTarget: Invalid World"));
		return nullptr;
	}

	// Compute center + ideal capture position
	FVector2D Center2D = ContentBounds.GetCenter();
	FVector2D Extents2D = ContentBounds.GetSize() * 0.5f + FVector2D(Padding, Padding);

	FVector CaptureTarget = FVector(Center2D.X, Center2D.Y, 0.f);
	FVector CaptureLocation = CaptureTarget + FVector(0.f, -Extents2D.Y * 2.f, Extents2D.Y * 2.f); // top-down view

	int32 MaxDim = FMath::CeilToInt(FMath::Max(Extents2D.X, Extents2D.Y) * 2.f);
	int32 Resolution = FMath::Clamp(FMath::RoundToInt(static_cast<float>(MaxDim)), 512, 4096);

	// Create fresh every time for now (optional: make it persistent)
	SystemOverviewRenderTarget = SystemMapUtils::CreateSystemOverviewRenderTarget(
		World,
		CaptureLocation,
		CaptureTarget,
		Resolution,
		TEXT("SystemOverview_Dynamic")
	);

	return SystemOverviewRenderTarget;
}


