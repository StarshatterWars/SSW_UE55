/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (C) 2025–2026.
    All Rights Reserved.

    SUBSYSTEM:    StarshatterWars (Unreal Engine)
    FILE:         StarshatterVideoSubsystem.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Implements Unreal-native video configuration handling
    while preserving the legacy Starshatter video pipeline.

    Adds:
      - ApplySettingsToRuntime() for UI + boot usage
      - static Get(...) convenience accessor
*/

#include "StarshatterVideoSubsystem.h"

#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "GameStructs.h"
#include "HAL/PlatformFilemanager.h"

// If you have the ported Starshatter singleton available:
#include "Starshatter.h"

// +--------------------------------------------------------------------+
// Convenience Accessor
// +--------------------------------------------------------------------+

UStarshatterVideoSubsystem* UStarshatterVideoSubsystem::Get(const UObject* WorldContextObject)
{
    if (!WorldContextObject)
        return nullptr;

    const UWorld* World = WorldContextObject->GetWorld();
    if (!World)
        return nullptr;

    UGameInstance* GI = World->GetGameInstance();
    if (!GI)
        return nullptr;

    return GI->GetSubsystem<UStarshatterVideoSubsystem>();
}

// +--------------------------------------------------------------------+
// Subsystem Lifecycle
// +--------------------------------------------------------------------+

void UStarshatterVideoSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Load video configuration on startup:
    LoadVideoConfig(TEXT("video.cfg"), true);
}

void UStarshatterVideoSubsystem::Deinitialize()
{
    // Optional: persist settings on shutdown
    // SaveVideoConfig(TEXT("video.cfg"));

    Super::Deinitialize();
}

// +--------------------------------------------------------------------+
// Path Utilities
// +--------------------------------------------------------------------+

FString UStarshatterVideoSubsystem::GetDefaultConfigDir() const
{
    return FPaths::Combine(
        FPaths::ProjectSavedDir(),
        TEXT("Starshatter"),
        TEXT("Config")
    );
}

FString UStarshatterVideoSubsystem::ResolveConfigPath(const FString& InRelativeOrAbsolutePath) const
{
    if (FPaths::IsRelative(InRelativeOrAbsolutePath))
    {
        return FPaths::Combine(
            GetDefaultConfigDir(),
            InRelativeOrAbsolutePath
        );
    }

    return InRelativeOrAbsolutePath;
}

// +--------------------------------------------------------------------+
// Load / Save
// +--------------------------------------------------------------------+

bool UStarshatterVideoSubsystem::LoadVideoConfig(const FString& InRelativeOrAbsolutePath, bool bCreateIfMissing)
{
    const FString AbsPath = ResolveConfigPath(InRelativeOrAbsolutePath);
    const FString Dir = FPaths::GetPath(AbsPath);

    IPlatformFile& PF = FPlatformFileManager::Get().GetPlatformFile();
    PF.CreateDirectoryTree(*Dir);

    FString Text;
    if (!FFileHelper::LoadFileToString(Text, *AbsPath))
    {
        if (bCreateIfMissing)
            SaveVideoConfig(InRelativeOrAbsolutePath);

        return false;
    }

    TArray<FString> Lines;
    Text.ParseIntoArrayLines(Lines);

    TMap<FString, FString> KV;

    for (FString Line : Lines)
    {
        Line = Line.TrimStartAndEnd();
        if (Line.IsEmpty())
            continue;

        // Skip header lines like "VIDEO"
        if (Line.Equals(TEXT("VIDEO"), ESearchCase::IgnoreCase))
            continue;

        // Skip comments
        if (Line.StartsWith(TEXT("#")) || Line.StartsWith(TEXT("//")))
            continue;

        FString Key, Value;

        // Support BOTH "key=value" AND legacy "key: value"
        if (Line.Split(TEXT("="), &Key, &Value) || Line.Split(TEXT(":"), &Key, &Value))
        {
            KV.Add(Key.TrimStartAndEnd(), Value.TrimStartAndEnd());
        }
    }

    auto Get = [&](const TCHAR* K) -> const FString*
        {
            return KV.Find(FString(K));
        };

    // ----------------------------------------------------------------
    // Map config keys -> FStarshatterVideoConfig (GameStructs version)
    // ----------------------------------------------------------------

    if (const FString* V = Get(TEXT("width")))                  ParseInt(*V, CurrentConfig.Width);
    if (const FString* V = Get(TEXT("height")))                 ParseInt(*V, CurrentConfig.Height);
    if (const FString* V = Get(TEXT("fullscreen")))             ParseBool(*V, CurrentConfig.bFullscreen);

    if (const FString* V = Get(TEXT("max_tex")))                ParseInt(*V, CurrentConfig.MaxTextureSize);
    if (const FString* V = Get(TEXT("gamma")))                  ParseInt(*V, CurrentConfig.GammaLevel);

    if (const FString* V = Get(TEXT("shadows")))                ParseBool(*V, CurrentConfig.bShadows);
    if (const FString* V = Get(TEXT("spec_maps")))              ParseBool(*V, CurrentConfig.bSpecularMaps);
    if (const FString* V = Get(TEXT("bump_maps")))              ParseBool(*V, CurrentConfig.bBumpMaps);

    if (const FString* V = Get(TEXT("flare")))                  ParseBool(*V, CurrentConfig.bLensFlare);
    if (const FString* V = Get(TEXT("corona")))                 ParseBool(*V, CurrentConfig.bCorona);
    if (const FString* V = Get(TEXT("nebula")))                 ParseBool(*V, CurrentConfig.bNebula);

    if (const FString* V = Get(TEXT("dust")))                   ParseInt(*V, CurrentConfig.DustLevel);

    if (const FString* V = Get(TEXT("terrain_detail_level")))   ParseInt(*V, CurrentConfig.TerrainDetailIndex);
    if (const FString* V = Get(TEXT("terrain_texture_enable"))) ParseBool(*V, CurrentConfig.bTerrainTextures);

    // Sanitize a couple of known ranges:
    CurrentConfig.GammaLevel = FMath::Clamp(CurrentConfig.GammaLevel, 32, 224);
    CurrentConfig.DustLevel = FMath::Max(0, CurrentConfig.DustLevel);

    return true;
}

bool UStarshatterVideoSubsystem::SaveVideoConfig(const FString& InRelativeOrAbsolutePath) const
{
    const FString AbsPath = ResolveConfigPath(InRelativeOrAbsolutePath);
    const FString Dir = FPaths::GetPath(AbsPath);

    IPlatformFile& PF = FPlatformFileManager::Get().GetPlatformFile();
    PF.CreateDirectoryTree(*Dir);

    TArray<FString> Lines;

    Lines.Add(TEXT("VIDEO"));
    Lines.Add(TEXT(""));

    WriteLine(Lines, TEXT("width"), FString::FromInt(CurrentConfig.Width));
    WriteLine(Lines, TEXT("height"), FString::FromInt(CurrentConfig.Height));
    WriteLine(Lines, TEXT("depth"), TEXT("32"));
    Lines.Add(TEXT(""));

    WriteLine(Lines, TEXT("max_tex"), FString::FromInt(CurrentConfig.MaxTextureSize));
    WriteLine(Lines, TEXT("primary3D"), TEXT("true"));
    WriteLine(Lines, TEXT("gamma"), FString::FromInt(FMath::Clamp(CurrentConfig.GammaLevel, 32, 224)));
    Lines.Add(TEXT(""));

    WriteLine(Lines, TEXT("terrain_detail_level"), FString::FromInt(CurrentConfig.TerrainDetailIndex));
    WriteLine(Lines, TEXT("terrain_texture_enable"), CurrentConfig.bTerrainTextures ? TEXT("true") : TEXT("false"));
    Lines.Add(TEXT(""));

    WriteLine(Lines, TEXT("shadows"), CurrentConfig.bShadows ? TEXT("true") : TEXT("false"));
    WriteLine(Lines, TEXT("spec_maps"), CurrentConfig.bSpecularMaps ? TEXT("true") : TEXT("false"));
    WriteLine(Lines, TEXT("bump_maps"), CurrentConfig.bBumpMaps ? TEXT("true") : TEXT("false"));

    // Legacy supports bias but you removed it from your struct; keep a default:
    WriteLine(Lines, TEXT("bias"), TEXT("0.0"));
    Lines.Add(TEXT(""));

    WriteLine(Lines, TEXT("flare"), CurrentConfig.bLensFlare ? TEXT("true") : TEXT("false"));
    WriteLine(Lines, TEXT("corona"), CurrentConfig.bCorona ? TEXT("true") : TEXT("false"));
    WriteLine(Lines, TEXT("nebula"), CurrentConfig.bNebula ? TEXT("true") : TEXT("false"));
    WriteLine(Lines, TEXT("dust"), FString::FromInt(CurrentConfig.DustLevel));

    return FFileHelper::SaveStringToFile(
        FString::Join(Lines, TEXT("\n")) + TEXT("\n"),
        *AbsPath
    );
}

// +--------------------------------------------------------------------+
// Config Access
// +--------------------------------------------------------------------+

void UStarshatterVideoSubsystem::SetConfig(const FStarshatterVideoConfig& NewConfig)
{
    CurrentConfig = NewConfig;

    CurrentConfig.GammaLevel = FMath::Clamp(CurrentConfig.GammaLevel, 32, 224);
    CurrentConfig.DustLevel = FMath::Max(0, CurrentConfig.DustLevel);

    // Optional clamps if you want:
    CurrentConfig.Width = FMath::Max(320, CurrentConfig.Width);
    CurrentConfig.Height = FMath::Max(200, CurrentConfig.Height);
}

// +--------------------------------------------------------------------+
// Deferred Change Handling
// +--------------------------------------------------------------------+

void UStarshatterVideoSubsystem::RequestChangeVideo(const FStarshatterVideoConfig& NewPendingConfig)
{
    PendingConfig = NewPendingConfig;
    bPendingChange = true;

    OnVideoChangeRequested.Broadcast(PendingConfig);
}

bool UStarshatterVideoSubsystem::ConsumePendingChange(FStarshatterVideoConfig& OutPending)
{
    if (!bPendingChange)
        return false;

    OutPending = PendingConfig;
    CurrentConfig = PendingConfig;
    bPendingChange = false;

    return true;
}

// +--------------------------------------------------------------------+
// Runtime Apply Hook
// +--------------------------------------------------------------------+

void UStarshatterVideoSubsystem::ApplySettingsToRuntime()
{
    // Persist:
    SaveVideoConfig(TEXT("video.cfg"));

    if (Starshatter* Stars = Starshatter::GetInstance())
    {
        // If your core can tell current mode, compare it.
        // For now, treat fullscreen/width/height as "mode change" triggers:
        const bool bModeChange =
            CurrentConfig.bFullscreen != Stars->GetVideo()->IsFullScreen() ||   // if you have this access
            CurrentConfig.Width != Stars->GetVideo()->Width() ||
            CurrentConfig.Height != Stars->GetVideo()->Height();

        if (bModeChange)
        {
            Stars->RequestChangeVideo();
        }
        else
        {
            Stars->LoadVideoConfig("video.cfg");
        }
    }
    else
    {
        OnVideoChangeRequested.Broadcast(CurrentConfig);
    }
}


// +--------------------------------------------------------------------+
// Parsing Helpers
// +--------------------------------------------------------------------+

bool UStarshatterVideoSubsystem::ParseBool(const FString& Value, bool& OutBool)
{
    const FString V = Value.TrimStartAndEnd().ToLower();
    OutBool = (V == TEXT("1") || V == TEXT("true") || V == TEXT("yes") || V == TEXT("on"));
    return true;
}

bool UStarshatterVideoSubsystem::ParseInt(const FString& Value, int32& OutInt)
{
    OutInt = FCString::Atoi(*Value);
    return true;
}

bool UStarshatterVideoSubsystem::ParseFloat(const FString& Value, float& OutFloat)
{
    OutFloat = FCString::Atof(*Value);
    return true;
}

void UStarshatterVideoSubsystem::WriteLine(TArray<FString>& Lines, const FString& Key, const FString& Value)
{
    Lines.Add(Key + TEXT(": ") + Value);
}