#include "StarshatterBootSubsystem.h"

#include "Engine/GameInstance.h"

// Include actual subsystem headers:
#include "FontManagerSubsystem.h"
#include "StarshatterAudioSubsystem.h"
#include "StarshatterVideoSubsystem.h"
#include "StarshatterControlsSubsystem.h"
#include "StarshatterSettingsSaveSubsystem.h"

void UStarshatterBootSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    BootSettings();
    BootFonts();
    BootAudio();
    BootVideo();
    BootControls();
}

void UStarshatterBootSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

void UStarshatterBootSubsystem::BootSettings()
{
    UGameInstance* GI = GetGameInstance();
    if (!GI)
        return;

    if (UStarshatterSettingsSaveSubsystem* SaveSS = GI->GetSubsystem<UStarshatterSettingsSaveSubsystem>())
    {
        SaveSS->LoadOrCreate();
    }
}

void UStarshatterBootSubsystem::BootFonts()
{
    UGameInstance* GI = GetGameInstance();
    if (!GI)
        return;

    if (UFontManagerSubsystem* FontSS = GI->GetSubsystem<UFontManagerSubsystem>())
    {
        // Optional if/when you add methods:
        // FontSS->LoadFontConfig();
        // FontSS->ApplyToRuntimeFonts();
    }
}

void UStarshatterBootSubsystem::BootAudio()
{
    UGameInstance* GI = GetGameInstance();
    if (!GI)
        return;

    if (UStarshatterSettingsSaveSubsystem* SaveSS = GI->GetSubsystem<UStarshatterSettingsSaveSubsystem>())
    {
        SaveSS->LoadOrCreate();

        if (UStarshatterSettingsSaveGame* SG = SaveSS->GetSettings())
        {
            if (UStarshatterAudioSubsystem* AudioSS = GI->GetSubsystem<UStarshatterAudioSubsystem>())
            {
                AudioSS->LoadFromSaveGame(SG);
                AudioSS->ApplySettingsToRuntime();
            }
        }
    }
}

void UStarshatterBootSubsystem::BootVideo()
{
    UGameInstance* GI = GetGameInstance();
    if (!GI)
        return;

    UStarshatterSettingsSaveSubsystem* SaveSS = GI->GetSubsystem<UStarshatterSettingsSaveSubsystem>();
    UStarshatterVideoSubsystem* VideoSS = GI->GetSubsystem<UStarshatterVideoSubsystem>();

    if (!SaveSS || !VideoSS)
        return;

    // BootSettings() already called LoadOrCreate(), so just use the cached settings:
    if (UStarshatterSettingsSaveGame* SG = SaveSS->GetSettings())
    {
        VideoSS->LoadFromSaveGame(SG);
        VideoSS->ApplySettingsToRuntime();
    }
    else
    {
        // fallback if something went sideways:
        VideoSS->LoadVideoConfig(TEXT("video.cfg"), true);
        VideoSS->ApplySettingsToRuntime();
    }
}

void UStarshatterBootSubsystem::BootControls()
{
    UGameInstance* GI = GetGameInstance();
    if (!GI) return;

    UStarshatterSettingsSaveSubsystem* SaveSS = GI->GetSubsystem<UStarshatterSettingsSaveSubsystem>();
    UStarshatterControlsSubsystem* ControlsSS = GI->GetSubsystem<UStarshatterControlsSubsystem>();
    if (!SaveSS || !ControlsSS) return;

    SaveSS->LoadOrCreate();

    if (UStarshatterSettingsSaveGame* SG = SaveSS->GetSettings())
    {
        ControlsSS->LoadFromSaveGame(SG);
        ControlsSS->ApplySettingsToRuntime(this);
    }
    else
    {
        ControlsSS->ApplySettingsToRuntime(this);
    }
}