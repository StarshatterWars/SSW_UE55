/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    nGenEx.lib (ported to Unreal)
    FILE:         FadeView.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UFadeView
    - UUserWidget-based fade controller view (inherits from UView)
    - Runs the legacy fade state machine in NativeTick
    - Writes fade to UFadeSubsystem (no direct rendering)
*/

#pragma once

#include "CoreMinimal.h"
#include "Math/Color.h"
#include "View.h"            /
#include "GameStructs.h"
#include "FadeView.generated.h"

class UFadeSubsystem;

UCLASS()
class STARSHATTERWARS_API UFadeView : public UView
{
    GENERATED_BODY()

public:
    static const char* TYPENAME() { return "FadeView"; }

    UFadeView(const FObjectInitializer& ObjectInitializer);
    UFadeView(Window* c, double fade_in = 1, double fade_out = 1, double hold_time = 4);

    // --- Legacy-ish API ---
    bool Done() const { return State == EFadeState::StateDone; }
    bool Holding() const { return State == EFadeState::StateHold; }

    void FastFade(int32 FadeFast);
    void FadeIn(double InFadeInSeconds);
    void FadeOut(double InFadeOutSeconds);
    void StopHold();

    // Writes fade into the subsystem:
    void SetFade(double Fade01, const FColor& Color, int32 BuildShade = 0);

    // Convenience initializer (optional):
    void InitFade(double InFadeInSeconds = 1.0, double InFadeOutSeconds = 1.0, double InHoldSeconds = 4.0);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    UFadeSubsystem* GetFadeSubsystem() const;
    void AdvanceStateMachine(double DeltaSeconds);

private:
    // Timings (seconds):
    double FadeInSeconds = 1.0;
    double FadeOutSeconds = 1.0;
    double HoldSeconds = 4.0;

    // Running:
    double StepTimeSeconds = 0.0;
    int32  Fast = 1;              // legacy: kept for compatibility (you can decide how to use)
    EFadeState State = EFadeState::StateStart;

    // Default fade color (legacy used black):
    FColor FadeColor = FColor::Black;
};
