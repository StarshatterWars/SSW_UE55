// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "GalaxyManager.h"
#include "Kismet/GameplayStatics.h"
#include "../Foundation/PlanetUtils.h"
#include "../Foundation/StarUtils.h"
#include "../Foundation/SystemMapUtils.h"
#include "../Foundation/MoonUtils.h"
#include "Slate/WidgetRenderer.h"
#include "Blueprint/UserWidget.h"
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

	if (!SystemOverviewRenderTarget)
	{
		UE_LOG(LogTemp, Error, TEXT("SystemOverviewRT was NOT created"));
		return nullptr;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SystemOverviewRT created: %s [%p]"), *SystemOverviewRenderTarget->GetName(), SystemOverviewRenderTarget);

		UE_LOG(LogTemp, Warning,
			TEXT("SystemOverviewRT Size: %d x %d | Format: %d"),
			SystemOverviewRenderTarget->SizeX,
			SystemOverviewRenderTarget->SizeY,
			(int32)SystemOverviewRenderTarget->RenderTargetFormat
		);
	}

	return SystemOverviewRenderTarget;
}

UTextureRenderTarget2D* UGalaxyManager::RenderWidgetToTarget(UUserWidget* Widget, int32 Width, int32 Height, float Scale)
{
	if (!Widget)
	{
		UE_LOG(LogTemp, Error, TEXT("[RenderWidgetToTarget] Null widget"));
		return nullptr;
	}

	// Create renderer once
	if (!WidgetRenderer.IsValid())
	{
		WidgetRenderer = MakeShared<FWidgetRenderer>(true);
	}

	UTextureRenderTarget2D* RT = NewObject<UTextureRenderTarget2D>(this);
	RT->RenderTargetFormat = RTF_RGBA8;
	RT->InitAutoFormat(Width, Height);
	RT->ClearColor = FLinearColor::Transparent;
	RT->UpdateResourceImmediate();

	WidgetRenderer.Get()->DrawWidget(
		RT,
		Widget->TakeWidget(),
		FVector2D(Width, Height),
		Scale
	);

	UE_LOG(LogTemp, Log, TEXT("[RenderWidgetToTarget] Rendered %s to %dx%d"), *Widget->GetName(), Width, Height);
	return RT;
}

