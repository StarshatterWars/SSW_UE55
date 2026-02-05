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

    if (UStarshatterAudioSubsystem* AudioSS = GI->GetSubsystem<UStarshatterAudioSubsystem>())
    {
        // This one exists in your codebase:
        AudioSS->LoadAudioConfig();

        // If your Audio subsystem also has an apply method, call it here.
        // (Leave commented if not present to avoid C2039.)
        // AudioSS->ApplySettingsToRuntime();
        // AudioSS->ApplyToRuntimeAudio();
    }
}

void UStarshatterBootSubsystem::BootVideo()
{
    UGameInstance* GI = GetGameInstance();
    if (!GI)
        return;

    if (UStarshatterVideoSubsystem* VideoSS = GI->GetSubsystem<UStarshatterVideoSubsystem>())
    {
        // You have this:
        VideoSS->LoadVideoConfig(TEXT("video.cfg"), true);

        // And you have this as NO-ARG:
        VideoSS->ApplySettingsToRuntime();
    }
}

void UStarshatterBootSubsystem::BootControls()
{
    UGameInstance* GI = GetGameInstance();
    if (!GI)
        return;

    if (UStarshatterControlsSubsystem* ControlsSS = GI->GetSubsystem<UStarshatterControlsSubsystem>())
    {
        // If you have a legacy load method, call it here.
        // If you don't have one yet, skip loading and just apply defaults.
        //
        // Examples (COMMENTED OUT until your class actually has them):
        // ControlsSS->LoadControlsConfig(TEXT("key.cfg"), true);
        // ControlsSS->LoadKeyConfig();

        // Your Controls Apply currently REQUIRES an argument.
        // Pass a world context that always exists in subsystem:
        ControlsSS->ApplySettingsToRuntime(this);
    }
}
