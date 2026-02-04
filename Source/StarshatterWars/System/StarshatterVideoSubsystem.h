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
    Starshatter Video Subsystem

    Unreal-native replacement for legacy video configuration handling.
    This subsystem owns:
      - video.cfg load/save
      - current video configuration state
      - pending video change requests

    The subsystem DOES NOT reset the renderer directly.
    Instead, it signals intent and allows the legacy Starshatter
    core or renderer layer to perform the actual reset.
*/

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "StarshatterVideoSubsystem.generated.h"

// +--------------------------------------------------------------------+
// Video Configuration Model
// +--------------------------------------------------------------------+

/*
    Represents the full runtime video configuration.
    This structure mirrors the legacy video.cfg fields
    and can be expanded incrementally during the port.
*/
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Video")
    float Gamma = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Video")
    float DepthBias = 0.0f;
};

// +--------------------------------------------------------------------+
// Delegates
// +--------------------------------------------------------------------+

/*
    Broadcast when a video change is requested.
    The renderer or Starshatter core may subscribe
    and perform the actual reset.
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnStarshatterVideoChangeRequested,
    const FStarshatterVideoConfig&,
    PendingConfig
);

// +--------------------------------------------------------------------+
// Video Subsystem
// +--------------------------------------------------------------------+

/*
    UStarshatterVideoSubsystem

    GameInstance-level subsystem responsible for:
      - loading/saving video.cfg
      - storing current video settings
      - managing deferred video changes

    Designed to coexist with legacy Starshatter
    ChangeVideo / ResizeVideo logic during migration.
*/
UCLASS()
class STARSHATTERWARS_API UStarshatterVideoSubsystem
    : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // +----------------------------------------------------------------
    // UGameInstanceSubsystem Interface
    // +----------------------------------------------------------------

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

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

    // +----------------------------------------------------------------
    // Configuration Access
    // +----------------------------------------------------------------

    const FStarshatterVideoConfig& GetConfig() const { return CurrentConfig; }
    void SetConfig(const FStarshatterVideoConfig& NewConfig);

    // +----------------------------------------------------------------
    // Deferred Change Handling
    // +----------------------------------------------------------------

    void RequestChangeVideo(const FStarshatterVideoConfig& NewPendingConfig);
    bool HasPendingChange() const { return bPendingChange; }

    bool ConsumePendingChange(FStarshatterVideoConfig& OutPending);

    // +----------------------------------------------------------------
    // Events
    // +----------------------------------------------------------------

    UPROPERTY(BlueprintAssignable, Category = "Starshatter|Video")
    FOnStarshatterVideoChangeRequested OnVideoChangeRequested;

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
