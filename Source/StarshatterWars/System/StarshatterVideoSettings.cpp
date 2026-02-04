/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe (Unreal Port)
    FILE:         StarshatterVideoSettings.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Implements UStarshatterVideoSettings:
    config-backed load/save/sanitize and optional runtime apply stub.
*/

#include "StarshatterVideoSettings.h"

#include "Misc/ConfigCacheIni.h"

UStarshatterVideoSettings* UStarshatterVideoSettings::Get()
{
    return GetMutableDefault<UStarshatterVideoSettings>();
}

void UStarshatterVideoSettings::Load()
{
    ReloadConfig();
    Sanitize();
}

void UStarshatterVideoSettings::Save() const
{
    const_cast<UStarshatterVideoSettings*>(this)->SaveConfig();
}

int32 UStarshatterVideoSettings::ClampPow2Tex(int32 V)
{
    // Accept common legacy values only (extend as needed):
    static const int32 Allowed[] = { 64, 128, 256, 512, 1024, 2048, 4096, 8192 };

    int32 Best = Allowed[0];
    for (int32 A : Allowed)
    {
        Best = A;
        if (V <= A) break;
    }
    return Best;
}

int32 UStarshatterVideoSettings::ClampGamma16(int32 V)
{
    V = FMath::Clamp(V, 32, 224);
    const int32 Snapped = (int32)(FMath::RoundToInt((float)V / 16.0f) * 16);
    return FMath::Clamp(Snapped, 32, 224);
}

void UStarshatterVideoSettings::Sanitize()
{
    Width = FMath::Clamp(Width, 640, 8192);
    Height = FMath::Clamp(Height, 480, 8192);

    MaxTexSize = ClampPow2Tex(MaxTexSize);

    TerrainDetailLevel = FMath::Clamp(TerrainDetailLevel, 0, 6);
    Dust = FMath::Clamp(Dust, 0, 3);

    GammaLevel = ClampGamma16(GammaLevel);
    DepthBias = FMath::Clamp(DepthBias, -10.0f, 10.0f);
}

void UStarshatterVideoSettings::ApplyToRuntimeVideo(UObject* WorldContextObject) const
{
    // Stub-safe: wire this later.
    // Examples you can implement next (depending on how your port is structured):
    // - Apply gamma (Game::SetGammaLevel)
    // - Apply Terrain detail / texture toggles
    // - Apply Video feature toggles (shadows/spec/bump)
    // - Trigger renderer reset or a deferred “video change request”
    //
    // For now, keep it a no-op that compiles cleanly.
    (void)WorldContextObject;
}

// -------------------------
// Setters
// -------------------------

void UStarshatterVideoSettings::SetWidth(int32 V)
{
    Width = FMath::Clamp(V, 640, 8192);
}

void UStarshatterVideoSettings::SetHeight(int32 V)
{
    Height = FMath::Clamp(V, 480, 8192);
}

void UStarshatterVideoSettings::SetMaxTexSize(int32 V)
{
    MaxTexSize = ClampPow2Tex(V);
}

void UStarshatterVideoSettings::SetTerrainDetailLevel(int32 V)
{
    TerrainDetailLevel = FMath::Clamp(V, 0, 6);
}

void UStarshatterVideoSettings::SetDust(int32 V)
{
    Dust = FMath::Clamp(V, 0, 3);
}

void UStarshatterVideoSettings::SetGammaLevel(int32 V)
{
    GammaLevel = ClampGamma16(V);
}
