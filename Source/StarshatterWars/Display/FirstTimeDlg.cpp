/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         FirstTimeDlg.cpp
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    First-time player setup screen implementation.
*/

#include "FirstTimeDlg.h"

#include "GameStructs.h"

// Unreal
#include "Logging/LogMacros.h"
#include "Math/UnrealMathUtility.h"

// UMG
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/ComboBoxString.h"

// Starshatter
#include "PlayerCharacter.h"
#include "MenuScreen.h"
#include "Ship.h"
#include "Starshatter.h"
#include "KeyMap.h"
#include "Random.h"

DEFINE_LOG_CATEGORY_STATIC(LogFirstTimeDlg, Log, All);

// ------------------------------------------------------------

UFirstTimeDlg::UFirstTimeDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

// ------------------------------------------------------------

void UFirstTimeDlg::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (ApplyButton)
    {
        ApplyButton->OnClicked.AddDynamic(
            this,
            &UFirstTimeDlg::OnApplyClicked
        );
    }
}

// ------------------------------------------------------------

void UFirstTimeDlg::NativeConstruct()
{
    Super::NativeConstruct();

    if (NameEdit)
    {
        NameEdit->SetText(FText::FromString(TEXT("Noobie")));
    }
}

// ------------------------------------------------------------

void UFirstTimeDlg::NativeTick(
    const FGeometry& MyGeometry,
    float InDeltaTime
)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    ExecFrame();
}

// ------------------------------------------------------------

void UFirstTimeDlg::SetManager(UMenuScreen* InManager)
{
    Manager = InManager;
}

// ------------------------------------------------------------

void UFirstTimeDlg::ExecFrame()
{
    // Legacy dialog had no per-frame logic
}

// ------------------------------------------------------------

void UFirstTimeDlg::OnApplyClicked()
{
    Starshatter* stars = Starshatter::GetInstance();
    PlayerCharacter* player = PlayerCharacter::GetCurrentPlayer();

    if (!player)
    {
        UE_LOG(LogFirstTimeDlg, Warning, TEXT("No current player."));
        if (Manager)
            Manager->ShowMenuDlg();
        return;
    }

    // --------------------------------------------------------
    // Player name & password
    // --------------------------------------------------------
    if (NameEdit)
    {
        const FString PlayerName = NameEdit->GetText().ToString();

        const uint32 PassValue =
            (uint32)FMath::RandRange(0, 2000000000);

        const FString Password =
            FString::Printf(TEXT("%08x"), PassValue);

        FTCHARToUTF8 NameUtf8(*PlayerName);
        FTCHARToUTF8 PassUtf8(*Password);

        player->SetName(NameUtf8.Get());
        player->SetPassword(PassUtf8.Get());
    }

    // --------------------------------------------------------
    // Play style
    // --------------------------------------------------------
    if (PlaystyleCombo)
    {
        const int32 Sel = PlaystyleCombo->GetSelectedIndex();

        // ARCADE
        if (Sel == 0)
        {
            player->SetFlightModel(2);
            player->SetLandingModel(1);
            player->SetHUDMode(0);
            player->SetGunsight(1);

            if (stars)
            {
                KeyMap& keymap = stars->GetKeyMap();
                keymap.Bind(KEY_CONTROL_MODEL, 1, 0);
                keymap.SaveKeyMap("key.cfg", 256);
                stars->MapKeys();
            }

            Ship::SetControlModel(1);
        }
        // STANDARD / HARDCORE
        else
        {
            player->SetFlightModel(0);
            player->SetLandingModel(0);
            player->SetHUDMode(0);
            player->SetGunsight(0);

            if (stars)
            {
                KeyMap& keymap = stars->GetKeyMap();
                keymap.Bind(KEY_CONTROL_MODEL, 0, 0);
                keymap.SaveKeyMap("key.cfg", 256);
                stars->MapKeys();
            }

            Ship::SetControlModel(0);
        }
    }

    // --------------------------------------------------------
    // Experience level
    // --------------------------------------------------------
    if (ExperienceCombo &&
        ExperienceCombo->GetSelectedIndex() > 0)
    {
        player->SetRank(2);        // Lieutenant
        player->SetTrained(255);   // Fully trained
    }

    PlayerCharacter::Save();

    if (Manager)
        Manager->ShowMenuDlg();
}

FReply UFirstTimeDlg::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    UE_LOG(LogTemp, Warning, TEXT("FirstTimeDlg: MOUSE DOWN RECEIVED"));
    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}