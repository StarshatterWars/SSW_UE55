/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe (Unreal Port)
    FILE:         StarshatterControlsSettings.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Implements config-backed controls settings.
*/

#include "StarshatterControlsSettings.h"

#include "Misc/ConfigCacheIni.h"

UStarshatterControlsSettings* UStarshatterControlsSettings::Get()
{
    // Config-backed CDO:
    return GetMutableDefault<UStarshatterControlsSettings>();
}

void UStarshatterControlsSettings::Load()
{
    ReloadConfig();
    Sanitize();
}

void UStarshatterControlsSettings::Save() const
{
    const_cast<UStarshatterControlsSettings*>(this)->SaveConfig();
}

void UStarshatterControlsSettings::EnsureKeysSize(TArray<int32>& InOutKeys)
{
    if (InOutKeys.Num() != 256)
    {
        const int32 OldNum = InOutKeys.Num();
        InOutKeys.SetNum(256);

        // Initialize any new slots to 0 (unbound)
        for (int32 i = OldNum; i < 256; ++i)
            InOutKeys[i] = 0;
    }
}

void UStarshatterControlsSettings::Sanitize()
{
    ControlModel = FMath::Clamp(ControlModel, 0, 3);

    JoySelect = FMath::Max(0, JoySelect);
    JoyThrottle = FMath::Max(0, JoyThrottle);
    JoyRudder = FMath::Max(0, JoyRudder);
    JoySensitivity = FMath::Clamp(JoySensitivity, 0, 10);

    MouseSelect = FMath::Max(0, MouseSelect);
    MouseSensitivity = FMath::Clamp(MouseSensitivity, 0, 50);
    MouseInvert = MouseInvert ? 1 : 0;

    EnsureKeysSize(ActionKeys);

    // Action keys are "raw legacy key code integers" — keep them non-negative:
    for (int32& K : ActionKeys)
        K = FMath::Max(0, K);
}

// ---------------------------------------------------------------------
// Action table helpers
// ---------------------------------------------------------------------

int32 UStarshatterControlsSettings::GetActionKey(int32 ActionIndex) const
{
    if (!ActionKeys.IsValidIndex(ActionIndex))
        return 0;
    return ActionKeys[ActionIndex];
}

void UStarshatterControlsSettings::SetActionKey(int32 ActionIndex, int32 KeyCode)
{
    EnsureKeysSize(ActionKeys);

    if (!ActionKeys.IsValidIndex(ActionIndex))
        return;

    ActionKeys[ActionIndex] = FMath::Max(0, KeyCode);
}

void UStarshatterControlsSettings::SetActionKeys(const TArray<int32>& InKeys)
{
    ActionKeys = InKeys;
    EnsureKeysSize(ActionKeys);

    for (int32& K : ActionKeys)
        K = FMath::Max(0, K);
}

// ---------------------------------------------------------------------
// Setters (auto-sanitize)
// ---------------------------------------------------------------------

void UStarshatterControlsSettings::SetControlModel(int32 V)
{
    ControlModel = FMath::Clamp(V, 0, 3);
}

void UStarshatterControlsSettings::SetJoySelect(int32 V)
{
    JoySelect = FMath::Max(0, V);
}

void UStarshatterControlsSettings::SetJoyThrottle(int32 V)
{
    JoyThrottle = FMath::Max(0, V);
}

void UStarshatterControlsSettings::SetJoyRudder(int32 V)
{
    JoyRudder = FMath::Max(0, V);
}

void UStarshatterControlsSettings::SetJoySensitivity(int32 V)
{
    JoySensitivity = FMath::Clamp(V, 0, 10);
}

void UStarshatterControlsSettings::SetMouseSelect(int32 V)
{
    MouseSelect = FMath::Max(0, V);
}

void UStarshatterControlsSettings::SetMouseSensitivity(int32 V)
{
    MouseSensitivity = FMath::Clamp(V, 0, 50);
}

void UStarshatterControlsSettings::SetMouseInvert(int32 V)
{
    MouseInvert = (V != 0) ? 1 : 0;
}
