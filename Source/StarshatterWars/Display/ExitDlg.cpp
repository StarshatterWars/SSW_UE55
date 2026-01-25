/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         ExitDlg.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Exit / Credits Dialog (Unreal UUserWidget)

    FORM -> UMG mapping (recommended):
    - Title (id 100): UTextBlock* Title
    - Prompt (id 101): UTextBlock* Prompt
    - Credits scroller (id 201): URichTextBlock or UTextBlock inside a ScrollBox
      (this implementation uses UMultiLineEditableTextBox as a simple scroller)
    - Exit button (id 1): ApplyBtn (text "Exit")
    - Cancel button (id 2): CancelBtn
*/

#include "GameStructs.h"

#include "ExitDlg.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Input/Reply.h"
#include "InputCoreTypes.h"

// Starshatter core:
#include "Starshatter.h"
#include "Game.h"
#include "DataLoader.h"
#include "MusicManager.h"

// +--------------------------------------------------------------------+

static void CallManagerByName(UBaseScreen* manager, const TCHAR* fnName)
{
    if (!manager || !fnName) return;

    const FName fname(fnName);
    UFunction* fn = manager->FindFunction(fname);
    if (fn)
    {
        manager->ProcessEvent(fn, nullptr);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ExitDlg: Manager does not implement '%s'."), fnName);
    }
}

// +--------------------------------------------------------------------+

UExitDlg::UExitDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UExitDlg::NativeConstruct()
{
    Super::NativeConstruct();

    if (ApplyBtn)
    {
        ApplyBtn->OnClicked.AddDynamic(this, &UExitDlg::OnApplyClicked);
    }

    if (CancelBtn)
    {
        CancelBtn->OnClicked.AddDynamic(this, &UExitDlg::OnCancelClicked);
    }

    // Latch Escape for first tick after opening (matches legacy exit_latch behavior):
    exit_latch = true;

    // Enter credits mode (legacy: MusicDirector::CREDITS):
    MusicManager::SetMode(MusicManager::CREDITS);

    // Load credits text (legacy: credits.txt via DataLoader):
    DataLoader* loader = DataLoader::GetLoader();
    if (loader)
    {
        BYTE* block = nullptr;

        loader->SetDataPath(nullptr);
        loader->LoadBuffer("credits.txt", block, true);

        if (block && credits)
        {
            const char* cstr = reinterpret_cast<const char*>(block);
            credits->SetText(FText::FromString(UTF8_TO_TCHAR(cstr)));
        }

        loader->ReleaseBuffer(block);
    }

    // Ensure keyboard focus for Enter/Escape:
    SetKeyboardFocus();
}

void UExitDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Smooth scrolling approximation:
    // UMultiLineEditableTextBox doesn't expose a "top index" like RichTextBox.
    // If you swap to ScrollBox + RichTextBlock, you can implement proper smooth scroll.
    // Here we simply clear the latch after first frame so Escape works.
    exit_latch = false;
}

FReply UExitDlg::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    const FKey key = InKeyEvent.GetKey();

    if (key == EKeys::Enter || key == EKeys::Virtual_Accept)
    {
        Apply();
        return FReply::Handled();
    }

    if (key == EKeys::Escape || key == EKeys::Virtual_Back)
    {
        if (!exit_latch)
        {
            Cancel();
            return FReply::Handled();
        }
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

// +--------------------------------------------------------------------+
// Button handlers:

void UExitDlg::OnApplyClicked()
{
    Apply();
}

void UExitDlg::OnCancelClicked()
{
    Cancel();
}

// +--------------------------------------------------------------------+
// Operations:

void UExitDlg::Apply()
{
    Starshatter* stars = Starshatter::GetInstance();

    if (stars)
    {
        UE_LOG(LogTemp, Log, TEXT("Exit Confirmed."));
        stars->Exit();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ExitDlg: Starshatter instance not available; cannot exit."));
    }
}

void UExitDlg::Cancel()
{
    // Return to menu (legacy: manager->ShowMenuDlg()):
    CallManagerByName(manager, TEXT("ShowMenuDlg"));

    // Return to menu music (legacy: MusicDirector::MENU):
    MusicManager::SetMode(MusicManager::MENU);
}
