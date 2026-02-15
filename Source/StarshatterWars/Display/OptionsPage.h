/*=============================================================================
    Project:        Starshatter Wars
    Studio:         Fractal Dev Studios
    Copyright:      (c) 2025-2026.

    SUBSYSTEM:      StarshatterWars (Unreal Engine)
    FILE:           OptionsPage.h
    AUTHOR:         Carlos Bott

    OVERVIEW
    ========
    OptionsPage interface for OptionsScreen subpages.

    Purpose:
    - Allows UOptionsScreen to treat all subpages uniformly:
      LoadFromSettings(), ApplySettings(), SaveSettings(), CancelChanges()

    Notes:
    - These are BlueprintNativeEvents so subpages can implement in C++ or BP.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "OptionsPage.generated.h"

UINTERFACE(BlueprintType)
class STARSHATTERWARS_API UOptionsPage : public UInterface
{
    GENERATED_BODY()
};

class STARSHATTERWARS_API IOptionsPage
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Options")
    void LoadFromSettings();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Options")
    void ApplySettings();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Options")
    void SaveSettings();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Options")
    void CancelChanges();
};
#pragma once
