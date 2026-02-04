/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (C) 2025–2026.
    All Rights Reserved.

    SUBSYSTEM:    StarshatterWars (Unreal Engine)
    FILE:         StarshatterControlsSubsystem.cpp
    AUTHOR:       Carlos Bott
*/

#include "StarshatterControlsSubsystem.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

#include "StarshatterControlsSettings.h"

// Legacy:
#include "Starshatter.h"
#include "KeyMap.h"
#include "Ship.h"

UStarshatterControlsSubsystem* UStarshatterControlsSubsystem::Get(UObject* WorldContextObject)
{
    if (!WorldContextObject)
        return nullptr;

    UWorld* World = WorldContextObject->GetWorld();
    if (!World)
        return nullptr;

    UGameInstance* GI = World->GetGameInstance();
    if (!GI)
        return nullptr;

    return GI->GetSubsystem<UStarshatterControlsSubsystem>();
}

void UStarshatterControlsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UStarshatterControlsSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

Starshatter* UStarshatterControlsSubsystem::GetStars() const
{
    return Starshatter::GetInstance();
}

KeyMap* UStarshatterControlsSubsystem::GetLegacyKeyMap() const
{
    if (Starshatter* Stars = GetStars())
        return &Stars->GetKeyMap();

    return nullptr;
}

void UStarshatterControlsSubsystem::ApplySettingsToRuntime(UObject* /*WorldContextObject*/)
{
    // Settings source-of-truth:
    UStarshatterControlsSettings* S = UStarshatterControlsSettings::Get();
    if (!S)
        return;

    S->Load(); // ReloadConfig + Sanitize

    Starshatter* Stars = GetStars();
    KeyMap* KM = GetLegacyKeyMap();
    if (!Stars || !KM)
        return;

    // 1) Push "specials" (control model, mouse/joy configs)
    ApplySpecialsToLegacy(*KM, *S);

    // 2) Push action bindings (0..255 keys)
    ApplyActionBindingsToLegacy(*KM, *S);

    // 3) Persist legacy key.cfg for compatibility (optional but recommended during migration)
    KM->SaveKeyMap("key.cfg", 256);

    // 4) Rebuild runtime bindings (legacy)
    Stars->MapKeys();

    // 5) Apply control model to ship runtime
    Ship::SetControlModel(S->GetControlModel());
}

void UStarshatterControlsSubsystem::ApplySpecialsToLegacy(KeyMap& KM, const UStarshatterControlsSettings& S) const
{
    // IMPORTANT: KeyMap::Bind in your codebase appears to be Bind(action, key, shift)
    // (Your earlier compile log showed Bind() does NOT take 2 args.)
    // Keep shift=0 for now.

    KM.Bind(KEY_CONTROL_MODEL, S.GetControlModel(), 0);

    KM.Bind(KEY_JOY_SELECT, S.GetJoySelect(), 0);
    KM.Bind(KEY_JOY_THROTTLE, S.GetJoyThrottle(), 0);
    KM.Bind(KEY_JOY_RUDDER, S.GetJoyRudder(), 0);
    KM.Bind(KEY_JOY_SENSE, S.GetJoySensitivity(), 0);

    KM.Bind(KEY_MOUSE_SELECT, S.GetMouseSelect(), 0);
    KM.Bind(KEY_MOUSE_SENSE, S.GetMouseSensitivity(), 0);
    KM.Bind(KEY_MOUSE_INVERT, S.GetMouseInvert(), 0);
}

void UStarshatterControlsSubsystem::ApplyActionBindingsToLegacy(KeyMap& KM, const UStarshatterControlsSettings& S) const
{
    for (int32 ActionIndex = 0; ActionIndex < 256; ++ActionIndex)
    {
        const int32 KeyCode = S.GetActionKey(ActionIndex);
        if (KeyCode <= 0)
            continue;

        // Bind the action to the keycode (shift=0 default)
        KM.Bind(ActionIndex, KeyCode, 0);
    }
}
