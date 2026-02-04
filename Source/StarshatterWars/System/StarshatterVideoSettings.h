/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe (Unreal Port)
    FILE:         StarshatterVideoSettings.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UStarshatterVideoSettings
    - UE-native video settings container (config-backed).
    - Replaces legacy video.cfg pipeline for dialog + persistence.
    - Supports Load/Save/Sanitize and optional runtime apply.
    - Dialogs (VideoDlg) should ONLY touch this class for video state.
*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "StarshatterVideoSettings.generated.h"

UCLASS(Config = Game, DefaultConfig)
class STARSHATTERWARS_API UStarshatterVideoSettings : public UObject
{
    GENERATED_BODY()

public:
    // Singleton-like convenience accessor (CDO pattern)
    static UStarshatterVideoSettings* Get();

    // Reload from config (forces ini refresh)
    void Load();

    // Clamp/normalize
    void Sanitize();

    // Persist to config
    void Save() const;

    // Optional runtime apply hook (safe no-op until wired)
    void ApplyToRuntimeVideo(UObject* WorldContextObject) const;

public:
    // ----------------------------------------------------------------
    // Getters
    // ----------------------------------------------------------------
    int32 GetWidth() const { return Width; }
    int32 GetHeight() const { return Height; }
    bool  GetFullscreen() const { return bFullscreen; }

    int32 GetMaxTexSize() const { return MaxTexSize; }
    int32 GetTerrainDetailLevel() const { return TerrainDetailLevel; }      // legacy: 2..?
    bool  GetTerrainTextureEnabled() const { return bTerrainTextureEnable; }

    bool  GetShadows() const { return bShadows; }
    bool  GetSpecMaps() const { return bSpecMaps; }
    bool  GetBumpMaps() const { return bBumpMaps; }

    bool  GetLensFlare() const { return bLensFlare; }
    bool  GetCorona() const { return bCorona; }
    bool  GetNebula() const { return bNebula; }
    int32 GetDust() const { return Dust; }                                  // legacy: 0..N

    int32 GetGammaLevel() const { return GammaLevel; }                      // legacy: 32..224 step 16
    float GetDepthBias() const { return DepthBias; }

public:
    // ----------------------------------------------------------------
    // Setters (auto-sanitize where appropriate)
    // ----------------------------------------------------------------
    void SetWidth(int32 V);
    void SetHeight(int32 V);
    void SetFullscreen(bool bV) { bFullscreen = bV; }

    void SetMaxTexSize(int32 V);
    void SetTerrainDetailLevel(int32 V);
    void SetTerrainTextureEnabled(bool bV) { bTerrainTextureEnable = bV; }

    void SetShadows(bool bV) { bShadows = bV; }
    void SetSpecMaps(bool bV) { bSpecMaps = bV; }
    void SetBumpMaps(bool bV) { bBumpMaps = bV; }

    void SetLensFlare(bool bV) { bLensFlare = bV; }
    void SetCorona(bool bV) { bCorona = bV; }
    void SetNebula(bool bV) { bNebula = bV; }
    void SetDust(int32 V);

    void SetGammaLevel(int32 V);
    void SetDepthBias(float V) { DepthBias = V; }

private:
    static int32 ClampPow2Tex(int32 V);
    static int32 ClampGamma16(int32 V);

private:
    // ----------------------------------------------------------------
    // Stored in DefaultGame.ini (and Saved/Config overrides)
    // Keep names stable once shipped.
    // ----------------------------------------------------------------

    UPROPERTY(Config, EditAnywhere, Category = "Video")
    int32 Width = 1920;

    UPROPERTY(Config, EditAnywhere, Category = "Video")
    int32 Height = 1080;

    UPROPERTY(Config, EditAnywhere, Category = "Video")
    bool bFullscreen = true;

    UPROPERTY(Config, EditAnywhere, Category = "Video", meta = (ClampMin = "64", ClampMax = "8192"))
    int32 MaxTexSize = 2048;

    // Legacy maps Terrain::DetailLevel() typically (2..)
    UPROPERTY(Config, EditAnywhere, Category = "Video", meta = (ClampMin = "0", ClampMax = "6"))
    int32 TerrainDetailLevel = 2;

    UPROPERTY(Config, EditAnywhere, Category = "Video")
    bool bTerrainTextureEnable = true;

    UPROPERTY(Config, EditAnywhere, Category = "Video")
    bool bShadows = true;

    UPROPERTY(Config, EditAnywhere, Category = "Video")
    bool bSpecMaps = true;

    UPROPERTY(Config, EditAnywhere, Category = "Video")
    bool bBumpMaps = true;

    UPROPERTY(Config, EditAnywhere, Category = "Video")
    bool bLensFlare = true;

    UPROPERTY(Config, EditAnywhere, Category = "Video")
    bool bCorona = true;

    UPROPERTY(Config, EditAnywhere, Category = "Video")
    bool bNebula = true;

    UPROPERTY(Config, EditAnywhere, Category = "Video", meta = (ClampMin = "0", ClampMax = "3"))
    int32 Dust = 0;

    // Legacy gamma scale 32..224 (steps of 16)
    UPROPERTY(Config, EditAnywhere, Category = "Video", meta = (ClampMin = "32", ClampMax = "224"))
    int32 GammaLevel = 128;

    UPROPERTY(Config, EditAnywhere, Category = "Video")
    float DepthBias = 0.0f;
};
