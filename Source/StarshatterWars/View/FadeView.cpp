#include "FadeView.h"

// Subsystem:
#include "FadeSubsystem.h"

// Unreal:
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Logging/LogMacros.h"
#include "Math/UnrealMathUtility.h"

DEFINE_LOG_CATEGORY_STATIC(LogFadeView, Log, All);

UFadeView::UFadeView(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Safe defaults only. No external params here.
    fade_in = 0.0;
    fade_out = 0.0;
    hold_time = 0.0;
    step_time = 0.0;
    fast = true;
    time = 0.0;
    state = EFadeState::StateStart;
}

void UFadeView::Init(double InSeconds, double OutSeconds, double Hold)
{
    fade_in = InSeconds * 1000.0;
    fade_out = OutSeconds * 1000.0;
    hold_time = Hold * 1000.0;

    step_time = 0.0;
    fast = true;
    time = 0.0;
    state = EFadeState::StateStart;
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

void UFadeView::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
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

void UFadeView::SetFade(double Fade01, const FColor& Color, int32 BuildShade)
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



