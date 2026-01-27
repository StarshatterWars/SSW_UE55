#include "FadeView.h"

#include "Window.h"
#include "Game.h"

// Subsystem:
#include "FadeSubsystem.h"

// Unreal:
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogFadeView, Log, All);

FadeView::FadeView(Window* c, double in, double out, double hold)
	: UView(c)
	, fade_in(in * 1000.0)
	, fade_out(out * 1000.0)
	, hold_time(hold * 1000.0)
	, time(0.0)
	, step_time(0.0)
	, fast(1)
	, state(StateStart)
{
}

FadeView::~FadeView()
{
}

void FadeView::FadeIn(double in)
{
	fade_in = in * 1000.0;
}

void FadeView::FadeOut(double out)
{
	fade_out = out * 1000.0;
}

void FadeView::FastFade(int fade_fast)
{
	fast = fade_fast;
}

void FadeView::StopHold()
{
	UE_LOG(LogFadeView, VeryVerbose, TEXT("FadeView::StopHold()"));
	hold_time = 0.0;
}

UFadeSubsystem* FadeView::GetFadeSubsystem() const
{
	// You likely have a Window->GetWorld() or equivalent in your UE shims.
	// If not, adjust here to your project's canonical world access.
	UWorld* World = nullptr;

	if (window) {
		// If your Window shim has a world accessor, prefer it.
		// Otherwise, you may already have a global or Game::GetWorld() hook.
		World = window->GetWorld();
	}

	if (!World) {
		return nullptr;
	}

	UGameInstance* GI = World->GetGameInstance();
	if (!GI) {
		return nullptr;
	}

	return GI->GetSubsystem<UFadeSubsystem>();
}

void FadeView::SetFade(double f, const FColor& c, int build_shade)
{
	f = FMath::Clamp(f, 0.0, 1.0);

	if (UFadeSubsystem* FadeSS = GetFadeSubsystem())
	{
		FadeSS->SetFade(f, c, build_shade != 0);
	}
	else
	{
		// Non-fatal: you can still log it for early boot phases.
		UE_LOG(LogFadeView, VeryVerbose,
			TEXT("FadeView::SetFade skipped (FadeSubsystem not available) fade=%.3f"),
			f);
	}
}

void FadeView::Refresh()
{
	double msec = 0.0;

	if (state == StateStart) {
		time = Game::RealTime();
	}
	else if (state != StateDone) {
		const double new_time = Game::RealTime();
		msec = new_time - time;
		time = new_time;
	}

	switch (state) {

	case StateStart:
		if (fade_in > 0.0)
			SetFade(0.0, FColor::Black, 0);

		step_time = 0.0;
		state = State2;
		break;

	case State2:
		if (fade_in > 0.0)
			SetFade(0.0, FColor::Black, 0);

		step_time = 0.0;
		state = StateIn;
		break;

	case StateIn:
		if (step_time < fade_in) {
			const double fade =
				(fade_in > 0.0) ? (step_time / fade_in) : 1.0;

			SetFade(fade, FColor::Black, 0);
			step_time += msec;
		}
		else {
			SetFade(1.0, FColor::Black, 0);
			step_time = 0.0;
			state = StateHold;
		}
		break;

	case StateHold:
		if (step_time < hold_time) {
			step_time += msec;
		}
		else {
			step_time = 0.0;
			state = StateOut;
		}
		break;

	case StateOut:
		if (fade_out > 0.0) {
			if (step_time < fade_out) {
				const double fade =
					1.0 - (step_time / fade_out);

				SetFade(fade, FColor::Black, 0);
				step_time += msec;
			}
			else {
				SetFade(0.0, FColor::Black, 0);
				step_time = 0.0;
				state = StateDone;
			}
		}
		else {
			// Legacy behavior retained:
			SetFade(1.0, FColor::Black, 0);
			step_time = 0.0;
			state = StateDone;
		}
		break;

	case StateDone:/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    nGenEx.lib (ported to Unreal)
    FILE:         FadeView.cpp
    AUTHOR:       Carlos Bott
*/

#include "FadeView.h"

// Subsystem:
#include "FadeSubsystem.h"

// Unreal:
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Logging/LogMacros.h"
#include "Math/UnrealMathUtility.h"

        DEFINE_LOG_CATEGORY_STATIC(LogFadeView, Log, All);

        UFadeView::UFadeView(const FObjectInitializer & ObjectInitializer)
            : Super(ObjectInitializer)
        {
            // Defaults already set in header.
        }

        void UFadeView::NativeConstruct()
        {
            Super::NativeConstruct();

            // Start in the same “double start” pattern as the legacy code:
            State = EFadeState::StateStart;
            StepTimeSeconds = 0.0;

            // If you want an immediate “known” fade state on construct:
            if (FadeInSeconds > 0.0)
            {
                SetFade(0.0, FadeColor, 0);
            }
        }

        void UFadeView::NativeTick(const FGeometry & MyGeometry, float InDeltaTime)
        {
            Super::NativeTick(MyGeometry, InDeltaTime);

            if (State == EFadeState::StateDone)
                return;

            // Use real delta seconds. If you want “fast” to affect speed, apply it here:
            double DeltaSeconds = (double)InDeltaTime;

            // Optional legacy behavior: accelerate if Fast > 1
            if (Fast > 1)
            {
                DeltaSeconds *= (double)Fast;
            }

            AdvanceStateMachine(DeltaSeconds);
        }

        void UFadeView::InitFade(double InFadeInSeconds, double InFadeOutSeconds, double InHoldSeconds)
        {
            FadeInSeconds = FMath::Max(0.0, InFadeInSeconds);
            FadeOutSeconds = FMath::Max(0.0, InFadeOutSeconds);
            HoldSeconds = FMath::Max(0.0, InHoldSeconds);

            State = EFadeState::StateStart;
            StepTimeSeconds = 0.0;
        }

        void UFadeView::FadeIn(double InFadeInSeconds)
        {
            FadeInSeconds = FMath::Max(0.0, InFadeInSeconds);
        }

        void UFadeView::FadeOut(double InFadeOutSeconds)
        {
            FadeOutSeconds = FMath::Max(0.0, InFadeOutSeconds);
        }

        UFadeView::UFadeView(Window* c, double in, double out, double hold)
        {
            UView = c;
            fade_in = in * 1000;
            fade_out = out * 1000;
            hold_time = hold * 1000;
            step_time = 0;
            fast = 1;
            time = 0;
            state = StateStart;
        }

        void UFadeView::FastFade(int32 FadeFast)
        {
            Fast = FMath::Max(1, FadeFast);
        }

        void UFadeView::StopHold()
        {
            UE_LOG(LogFadeView, VeryVerbose, TEXT("UFadeView::StopHold()"));
            HoldSeconds = 0.0;
        }

        UFadeSubsystem* UFadeView::GetFadeSubsystem() const
        {
            UWorld* World = GetWorld();
            if (!World)
                return nullptr;

            UGameInstance* GI = World->GetGameInstance();
            if (!GI)
                return nullptr;

            return GI->GetSubsystem<UFadeSubsystem>();
        }

        void UFadeView::SetFade(double Fade01, const FColor & Color, int32 BuildShade)
        {
            const double Clamped = FMath::Clamp(Fade01, 0.0, 1.0);

            if (UFadeSubsystem* FadeSS = GetFadeSubsystem())
            {
                FadeSS->SetFade(Clamped, Color, BuildShade != 0);
            }
            else
            {
                UE_LOG(LogFadeView, VeryVerbose,
                    TEXT("UFadeView::SetFade skipped (FadeSubsystem not available) fade=%.3f"),
                    Clamped);
            }
        }

        void UFadeView::AdvanceStateMachine(double DeltaSeconds)
        {
            switch (State)
            {
            case EFadeState::StateStart:
                if (FadeInSeconds > 0.0)
                    SetFade(0.0, FadeColor, 0);

                StepTimeSeconds = 0.0;
                State = EFadeState::State2;
                break;

            case EFadeState::State2:
                if (FadeInSeconds > 0.0)
                    SetFade(0.0, FadeColor, 0);

                StepTimeSeconds = 0.0;
                State = EFadeState::StateIn;
                break;

            case EFadeState::StateIn:
                if (FadeInSeconds > 0.0)
                {
                    if (StepTimeSeconds < FadeInSeconds)
                    {
                        const double Fade = StepTimeSeconds / FadeInSeconds;
                        SetFade(Fade, FadeColor, 0);
                        StepTimeSeconds += DeltaSeconds;
                    }
                    else
                    {
                        SetFade(1.0, FadeColor, 0);
                        StepTimeSeconds = 0.0;
                        State = EFadeState::StateHold;
                    }
                }
                else
                {
                    SetFade(1.0, FadeColor, 0);
                    StepTimeSeconds = 0.0;
                    State = EFadeState::StateHold;
                }
                break;

            case EFadeState::StateHold:
                if (StepTimeSeconds < HoldSeconds)
                {
                    StepTimeSeconds += DeltaSeconds;
                }
                else
                {
                    StepTimeSeconds = 0.0;
                    State = EFadeState::StateOut;
                }
                break;

            case EFadeState::StateOut:
                if (FadeOutSeconds > 0.0)
                {
                    if (StepTimeSeconds < FadeOutSeconds)
                    {
                        const double Fade = 1.0 - (StepTimeSeconds / FadeOutSeconds);
                        SetFade(Fade, FadeColor, 0);
                        StepTimeSeconds += DeltaSeconds;
                    }
                    else
                    {
                        SetFade(0.0, FadeColor, 0);
                        StepTimeSeconds = 0.0;
                        State = EFadeState::StateDone;
                    }
                }
                else
                {
                    // Matches your legacy “else” branch (note: it sets fade to 1.0 then done)
                    SetFade(1.0, FadeColor, 0);
                    StepTimeSeconds = 0.0;
                    State = EFadeState::StateDone;
                }
                break;

            case EFadeState::StateDone:
            default:
                break;
            }
        }

	default:
		break;
	}
}
