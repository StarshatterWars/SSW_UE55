/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         ConfirmDlg.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    General-purpose confirmation dialog (Unreal port).

    UNREAL PORT:
    - Converted from FormWindow/AWEvent mapping to UBaseScreen (UUserWidget-derived).
    - Enter = Apply, Escape = Cancel.
    - Replaces ClientEvent(EID_USER_1) with an explicit delegate/callback.
*/

#include "ConfirmDlg.h"

// Input:
#include "Input/Reply.h"
#include "InputCoreTypes.h"

// Starshatter (ported core/gameplay):
#include "MenuScreen.h"
#include "Keyboard.h"

// Optional: hook your UI sound layer here (replaces Button::PlaySound):
// #include "HUDSounds.h"

UConfirmDlg::UConfirmDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UConfirmDlg::BindFormWidgets()
{
    // If you maintain ID-based bindings, map them here. IDs from classic:
    // 1 = apply, 2 = cancel, 100 = title, 101 = message
    BindButton(1, btn_apply);
    BindButton(2, btn_cancel);

    BindLabel(100, lbl_title);
    BindLabel(101, lbl_message);
}

void UConfirmDlg::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    RegisterControls();
}

void UConfirmDlg::NativeConstruct()
{
    Super::NativeConstruct();
    bExitLatch = true;
}

void UConfirmDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    ExecFrame();
}

FReply UConfirmDlg::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    const FKey Key = InKeyEvent.GetKey();

    if (Key == EKeys::Enter || Key == EKeys::Virtual_Accept)
    {
        OnApply();
        return FReply::Handled();
    }

    if (Key == EKeys::Escape || Key == EKeys::Virtual_Back)
    {
        OnCancel();
        return FReply::Handled();
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

// +--------------------------------------------------------------------+

void UConfirmDlg::RegisterControls()
{
    if (btn_apply)
    {
        btn_apply->OnClicked.RemoveAll(this);
        btn_apply->OnClicked.AddDynamic(this, &UConfirmDlg::OnApply);
    }

    if (btn_cancel)
    {
        btn_cancel->OnClicked.RemoveAll(this);
        btn_cancel->OnClicked.AddDynamic(this, &UConfirmDlg::OnCancel);
    }
}

void UConfirmDlg::ExecFrame()
{
    // Preserve classic behavior as a fallback if you keep Keyboard::KeyDown in your port layer:
    if (Keyboard::KeyDown(VK_RETURN))
        OnApply();

    if (Keyboard::KeyDown(VK_ESCAPE))
        OnCancel();

    // Latch release (optional, for parity with other dialogs you ported):
    bExitLatch = false;
}

// +--------------------------------------------------------------------+
// Legacy-ish API
// +--------------------------------------------------------------------+

void UConfirmDlg::SetOnApply(TFunction<void()> InOnApply)
{
    OnApplyCallback = MoveTemp(InOnApply);
}

FText UConfirmDlg::GetTitle() const
{
    return lbl_title ? lbl_title->GetText() : FText::GetEmpty();
}

void UConfirmDlg::SetTitle(const FText& InTitle)
{
    if (lbl_title)
        lbl_title->SetText(InTitle);
}

FText UConfirmDlg::GetMessage() const
{
    return lbl_message ? lbl_message->GetText() : FText::GetEmpty();
}

void UConfirmDlg::SetMessage(const FText& InMessage)
{
    if (lbl_message)
        lbl_message->SetText(InMessage);
}

void UConfirmDlg::Show()
{
    // In classic: played confirm sound if first shown; then FormWindow::Show() + SetFocus().
    // In UE: your manager should AddToViewport/SetVisibility; we leave this as a hook.
    // Example:
    // SetVisibility(ESlateVisibility::Visible);
    // SetKeyboardFocus();

    // Optional: play confirm open sound via your UE sound layer here.
}

// +--------------------------------------------------------------------+

void UConfirmDlg::OnApply()
{
    // Classic:
    // manager->HideConfirmDlg();
    // if (parent_control) parent_control->ClientEvent(EID_USER_1);

    if (manager)
    {
        // Wire this when your MenuScreen exposes it:
        // manager->HideConfirmDlg();
    }

    if (OnApplyCallback)
        OnApplyCallback();
}

void UConfirmDlg::OnCancel()
{
    // Classic: manager->HideConfirmDlg();

    if (manager)
    {
        // Wire this when your MenuScreen exposes it:
        // manager->HideConfirmDlg();
    }
}
