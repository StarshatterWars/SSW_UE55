/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (C) 2025–2026.
    All Rights Reserved.

    SUBSYSTEM:    StarshatterWars (Unreal Engine)
    FILE:         StarshatterVideoSubsystem.h
    AUTHOR:       Carlos Bott

    ORIGINAL DESIGN:
    John DiCamillo / Destroyer Studios LLC (video.cfg handling)

    OVERVIEW
    ========
    UStarshatterVideoSubsystem

    Unreal-native replacement for legacy video configuration handling.
    This subsystem owns:
      - video.cfg load/save
      - current video configuration state
      - pending video change requests
      - runtime-apply hook (calls into your ported Starshatter core)

    The subsystem DOES NOT reset the renderer directly.
    Instead, it either:
      - broadcasts intent (OnVideoChangeRequested), and/or
      - calls into Starshatter core if available (ApplySettingsToRuntime)
*/

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "StarshatterVideoSubsystem.generated.h"

// +--------------------------------------------------------------------+
// Video Configuration Model
// +--------------------------------------------------------------------+

USTRUCT(BlueprintType)
struct FStarshatterVideoConfig
{
    GENERATED_BODY()

    // Resolution:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Video")
    int32 Width = 1280;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Video")
    int32 Height = 720;

    // Display mode:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Video")
    bool bFullscreen = false;

    // Legacy effect toggles:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Video")
    bool bLensFlare = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Video")
    bool bCorona = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Video")
    bool bNebula = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Video")
    bool bDust = true;

    // Quality parameters:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Video")
    int32 MaxTexSize = 2048;

    // Gamma mapping:
    // Legacy uses 32..224 integer. We store 0..1 here by default.
    // You can change this later; keep stable once shipped.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Video")
    float Gamma = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Video")
    float DepthBias = 0.0f;
};

// +--------------------------------------------------------------------+
// Delegates
// +--------------------------------------------------------------------+

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnStarshatterVideoChangeRequested,
    const FStarshatterVideoConfig&,
    PendingConfig
);

// +--------------------------------------------------------------------+
// Video Subsystem
// +--------------------------------------------------------------------+

UCLASS()
class STARSHATTERWARS_API UStarshatterVideoSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // +----------------------------------------------------------------
    // Convenience Accessor
    // +----------------------------------------------------------------

    static UStarshatterVideoSubsystem* Get(const UObject* WorldContextObject);

public:
    // +----------------------------------------------------------------
    // UGameInstanceSubsystem Interface
    // +----------------------------------------------------------------

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

public:
    // +----------------------------------------------------------------
    // Configuration I/O
    // +----------------------------------------------------------------

    bool LoadVideoConfig(
        const FString& InRelativeOrAbsolutePath = TEXT("video.cfg"),
        bool bCreateIfMissing = true
    );

    bool SaveVideoConfig(
        const FString& InRelativeOrAbsolutePath = TEXT("video.cfg")
    ) const;

public:
    // +----------------------------------------------------------------
    // Configuration Access
    // +----------------------------------------------------------------

    const FStarshatterVideoConfig& GetConfig() const { return CurrentConfig; }
    void SetConfig(const FStarshatterVideoConfig& NewConfig);

public:
    // +----------------------------------------------------------------
    // Deferred Change Handling
    // +----------------------------------------------------------------

    void RequestChangeVideo(const FStarshatterVideoConfig& NewPendingConfig);
    bool HasPendingChange() const { return bPendingChange; }

    bool ConsumePendingChange(FStarshatterVideoConfig& OutPending);

public:
    // +----------------------------------------------------------------
    // Runtime Apply Hook
    // +----------------------------------------------------------------

    /*
        ApplySettingsToRuntime()

        Applies the CURRENT config to the runtime system.
        During migration, this typically calls into:
          Starshatter::GetInstance()->LoadVideoConfig(...)
        or triggers:
          Starshatter::GetInstance()->RequestChangeVideo()

        Keep this method no-arg so UI callsites are simple.
    */
    void ApplySettingsToRuntime();

public:
    // +----------------------------------------------------------------
    // Events
    // +----------------------------------------------------------------

    UPROPERTY(BlueprintAssignable, Category = "Starshatter|Video")
    FOnStarshatterVideoChangeRequested OnVideoChangeRequested;

public:
    // +----------------------------------------------------------------
    // Path Helpers
    // +----------------------------------------------------------------

    FString GetDefaultConfigDir() const;
    FString ResolveConfigPath(const FString& InRelativeOrAbsolutePath) const;

private:
    // +----------------------------------------------------------------
    // Internal State
    // +----------------------------------------------------------------

    FStarshatterVideoConfig CurrentConfig;
    FStarshatterVideoConfig PendingConfig;
    bool bPendingChange = false;

private:
    // +----------------------------------------------------------------
    // Parsing Helpers
    // +----------------------------------------------------------------

    static bool ParseBool(const FString& Value, bool& OutBool);
    static bool ParseInt(const FString& Value, int32& OutInt);
    static bool ParseFloat(const FString& Value, float& OutFloat);

    static void WriteLine(
        TArray<FString>& Lines,
        const FString& Key,
        const FString& Value
    );
};

