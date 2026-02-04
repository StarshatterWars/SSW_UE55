/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe (Unreal Port)
    FILE:         StarshatterControlsSettings.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UStarshatterControlsSettings
    - UE-native controls settings container (config-backed).
    - Replaces legacy key.cfg persistence for the *new* UI/settings pipeline.
    - Stores:
        * Special control options (control model, joystick/mouse options)
        * Full legacy action->key table (256 entries)
    - Supports Load/Save/Sanitize.
    - Runtime apply is handled by UStarshatterControlsSubsystem (NOT here).
*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "StarshatterControlsSettings.generated.h"

UCLASS(Config = Game, DefaultConfig)
class STARSHATTERWARS_API UStarshatterControlsSettings : public UObject
{
    GENERATED_BODY()

public:
    // Singleton-like convenience accessor (CDO):
    static UStarshatterControlsSettings* Get();

    // Force reload from ini:
    void Load();

    // Clamp to sane ranges + fix array size:
    void Sanitize();

    // Save current values back to ini:
    void Save() const;

public:
    // -----------------------------
    // Getters
    // -----------------------------
    int32 GetControlModel() const { return ControlModel; }

    int32 GetJoySelect() const { return JoySelect; }
    int32 GetJoyThrottle() const { return JoyThrottle; }
    int32 GetJoyRudder() const { return JoyRudder; }
    int32 GetJoySensitivity() const { return JoySensitivity; }

    int32 GetMouseSelect() const { return MouseSelect; }
    int32 GetMouseSensitivity() const { return MouseSensitivity; }
    int32 GetMouseInvert() const { return MouseInvert; }

    // Full 0..255 action-key table:
    const TArray<int32>& GetActionKeys() const { return ActionKeys; }

    int32 GetActionKey(int32 ActionIndex) const;
    void  SetActionKey(int32 ActionIndex, int32 KeyCode);

    // -----------------------------
    // Setters (auto-sanitize)
    // -----------------------------
    void SetControlModel(int32 V);

    void SetJoySelect(int32 V);
    void SetJoyThrottle(int32 V);
    void SetJoyRudder(int32 V);
    void SetJoySensitivity(int32 V);

    void SetMouseSelect(int32 V);
    void SetMouseSensitivity(int32 V);
    void SetMouseInvert(int32 V);

    void SetActionKeys(const TArray<int32>& InKeys);

private:
    static void EnsureKeysSize(TArray<int32>& InOutKeys);

private:
    // Stored in DefaultGame.ini (+ user Saved/Config overrides)
    // Keep property names stable once shipped.

    UPROPERTY(Config, EditAnywhere, Category = "Controls", meta = (ClampMin = "0", ClampMax = "3"))
    int32 ControlModel = 0;

    UPROPERTY(Config, EditAnywhere, Category = "Controls|Joystick", meta = (ClampMin = "0", ClampMax = "64"))
    int32 JoySelect = 0;

    UPROPERTY(Config, EditAnywhere, Category = "Controls|Joystick", meta = (ClampMin = "0", ClampMax = "64"))
    int32 JoyThrottle = 0;

    UPROPERTY(Config, EditAnywhere, Category = "Controls|Joystick", meta = (ClampMin = "0", ClampMax = "64"))
    int32 JoyRudder = 0;

    UPROPERTY(Config, EditAnywhere, Category = "Controls|Joystick", meta = (ClampMin = "0", ClampMax = "10"))
    int32 JoySensitivity = 5; // 0..10

    UPROPERTY(Config, EditAnywhere, Category = "Controls|Mouse", meta = (ClampMin = "0", ClampMax = "16"))
    int32 MouseSelect = 0;

    UPROPERTY(Config, EditAnywhere, Category = "Controls|Mouse", meta = (ClampMin = "0", ClampMax = "50"))
    int32 MouseSensitivity = 25; // 0..50

    UPROPERTY(Config, EditAnywhere, Category = "Controls|Mouse", meta = (ClampMin = "0", ClampMax = "1"))
    int32 MouseInvert = 0; // 0/1

    // Legacy 256-entry table: action index -> key code
    UPROPERTY(Config, EditAnywhere, Category = "Controls|Legacy")
    TArray<int32> ActionKeys;
};
