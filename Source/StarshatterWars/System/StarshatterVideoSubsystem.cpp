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
*/

#include "StarshatterVideoSubsystem.h"

#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"

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

FString UStarshatterVideoSubsystem::ResolveConfigPath(
    const FString& InRelativeOrAbsolutePath
) const
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

bool UStarshatterVideoSubsystem::LoadVideoConfig(
    const FString& InRelativeOrAbsolutePath,
    bool bCreateIfMissing
)
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
        if (Line.IsEmpty() || Line.StartsWith(TEXT("#")) || Line.StartsWith(TEXT("//")))
            continue;

        FString Key, Value;
        if (Line.Split(TEXT("="), &Key, &Value))
            KV.Add(Key.TrimStartAndEnd(), Value.TrimStartAndEnd());
    }

    auto Get = [&](const TCHAR* K) -> const FString*
        {
            return KV.Find(FString(K));
        };

    if (const FString* V = Get(TEXT("width")))        ParseInt(*V, CurrentConfig.Width);
    if (const FString* V = Get(TEXT("height")))       ParseInt(*V, CurrentConfig.Height);
    if (const FString* V = Get(TEXT("fullscreen")))   ParseBool(*V, CurrentConfig.bFullscreen);

    if (const FString* V = Get(TEXT("lens_flare")))   ParseBool(*V, CurrentConfig.bLensFlare);
    if (const FString* V = Get(TEXT("corona")))       ParseBool(*V, CurrentConfig.bCorona);
    if (const FString* V = Get(TEXT("nebula")))       ParseBool(*V, CurrentConfig.bNebula);
    if (const FString* V = Get(TEXT("dust")))         ParseBool(*V, CurrentConfig.bDust);

    if (const FString* V = Get(TEXT("max_tex_size"))) ParseInt(*V, CurrentConfig.MaxTexSize);
    if (const FString* V = Get(TEXT("gamma")))        ParseFloat(*V, CurrentConfig.Gamma);
    if (const FString* V = Get(TEXT("depth_bias")))   ParseFloat(*V, CurrentConfig.DepthBias);

    return true;
}

bool UStarshatterVideoSubsystem::SaveVideoConfig(
    const FString& InRelativeOrAbsolutePath
) const
{
    const FString AbsPath = ResolveConfigPath(InRelativeOrAbsolutePath);
    const FString Dir = FPaths::GetPath(AbsPath);

    IPlatformFile& PF = FPlatformFileManager::Get().GetPlatformFile();
    PF.CreateDirectoryTree(*Dir);

    TArray<FString> Lines;

    Lines.Add(TEXT("# Starshatter Wars video configuration"));
    Lines.Add(TEXT(""));

    WriteLine(Lines, TEXT("width"), FString::FromInt(CurrentConfig.Width));
    WriteLine(Lines, TEXT("height"), FString::FromInt(CurrentConfig.Height));
    WriteLine(Lines, TEXT("fullscreen"), CurrentConfig.bFullscreen ? TEXT("1") : TEXT("0"));
    Lines.Add(TEXT(""));

    WriteLine(Lines, TEXT("lens_flare"), CurrentConfig.bLensFlare ? TEXT("1") : TEXT("0"));
    WriteLine(Lines, TEXT("corona"), CurrentConfig.bCorona ? TEXT("1") : TEXT("0"));
    WriteLine(Lines, TEXT("nebula"), CurrentConfig.bNebula ? TEXT("1") : TEXT("0"));
    WriteLine(Lines, TEXT("dust"), CurrentConfig.bDust ? TEXT("1") : TEXT("0"));
    Lines.Add(TEXT(""));

    WriteLine(Lines, TEXT("max_tex_size"), FString::FromInt(CurrentConfig.MaxTexSize));
    WriteLine(Lines, TEXT("gamma"), FString::SanitizeFloat(CurrentConfig.Gamma));
    WriteLine(Lines, TEXT("depth_bias"), FString::SanitizeFloat(CurrentConfig.DepthBias));

    return FFileHelper::SaveStringToFile(
        FString::Join(Lines, TEXT("\n")) + TEXT("\n"),
        *AbsPath
    );
}

// +--------------------------------------------------------------------+
// Deferred Change Handling
// +--------------------------------------------------------------------+

void UStarshatterVideoSubsystem::RequestChangeVideo(
    const FStarshatterVideoConfig& NewPendingConfig
)
{
    PendingConfig = NewPendingConfig;
    bPendingChange = true;

    OnVideoChangeRequested.Broadcast(PendingConfig);
}

bool UStarshatterVideoSubsystem::ConsumePendingChange(
    FStarshatterVideoConfig& OutPending
)
{
    if (!bPendingChange)
        return false;

    OutPending = PendingConfig;
    CurrentConfig = PendingConfig;
    bPendingChange = false;

    return true;
}

// +--------------------------------------------------------------------+
// Parsing Helpers
// +--------------------------------------------------------------------+

bool UStarshatterVideoSubsystem::ParseBool(
    const FString& Value,
    bool& OutBool
)
{
    const FString V = Value.ToLower();
    OutBool = (V == TEXT("1") || V == TEXT("true") || V == TEXT("yes") || V == TEXT("on"));
    return true;
}

bool UStarshatterVideoSubsystem::ParseInt(
    const FString& Value,
    int32& OutInt
)
{
    OutInt = FCString::Atoi(*Value);
    return true;
}

bool UStarshatterVideoSubsystem::ParseFloat(
    const FString& Value,
    float& OutFloat
)
{
    OutFloat = FCString::Atof(*Value);
    return true;
}

void UStarshatterVideoSubsystem::WriteLine(
    TArray<FString>& Lines,
    const FString& Key,
    const FString& Value
)
{
    Lines.Add(Key + TEXT("=") + Value);
}
