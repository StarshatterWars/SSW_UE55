#include "StarshatterAudioSubsystem.h"

#include "StarshatterAudioSettings.h"

void UStarshatterAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    Settings = GetGameInstance()->GetSubsystem<UStarshatterAudioSettings>();
    if (Settings)
    {
        Settings->Load();
        Settings->ApplyToRuntimeAudio();
    }
}

void UStarshatterAudioSubsystem::Deinitialize()
{
    if (Settings)
    {
        Settings->Save();
    }

    Settings = nullptr;
    Super::Deinitialize();
}

void UStarshatterAudioSubsystem::ApplyAndSave()
{
    if (!Settings)
        return;

    Settings->Sanitize();
    Settings->ApplyToRuntimeAudio();
    Settings->Save();
}

void UStarshatterAudioSubsystem::ReloadAndApply()
{
    if (!Settings)
        return;

    Settings->Load();
    Settings->ApplyToRuntimeAudio();
}
