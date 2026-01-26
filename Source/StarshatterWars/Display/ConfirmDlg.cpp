/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         ConfirmDlg.cpp
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    UNREAL PORT:
    - Converted from FormWindow/AWEvent mapping to UBaseScreen (UUserWidget-derived).
    - Removes MemDebug and classic mapping macros.
*/

#include "ConfirmDlg.h"

// UMG:
#include "Components/Button.h"
#include "Components/TextBlock.h"

// Starshatter (ported core/gameplay):
#include "MenuScreen.h"

UConfirmDlg::UConfirmDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UConfirmDlg::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // BindFormWidgets() is called by UBaseScreen::NativeOnInitialized()
    // after FormMap.Reset(). We only need to hook click delegates here.
    RegisterControls();
}

void UConfirmDlg::NativeConstruct()
{
    Super::NativeConstruct();
}

void UConfirmDlg::RegisterControls()
{
    // Wire UBaseScreen dialog policy (Enter/Escape)
    ApplyButton = btn_apply;
    CancelButton = btn_cancel;

    if (btn_apply)
    {
        btn_apply->OnClicked.RemoveAll(this);
        btn_apply->OnClicked.AddDynamic(this, &UConfirmDlg::OnApplyClicked);
    }

    if (btn_cancel)
    {
        btn_cancel->OnClicked.RemoveAll(this);
        btn_cancel->OnClicked.AddDynamic(this, &UConfirmDlg::OnCancelClicked);
    }
}

void UConfirmDlg::Show()
{
    // In UE, the caller typically AddToViewport()/SetVisibility().
    // This keeps the legacy surface for parity.
    SetVisibility(ESlateVisibility::Visible);

    // Optional: ensure focus so Enter/Escape works consistently
    // (depends on how you manage focus across screens)
    SetKeyboardFocus();
}

FString UConfirmDlg::GetTitle() const
{
    return lbl_title ? lbl_title->GetText().ToString() : FString();
}

void UConfirmDlg::SetTitle(const FString& InTitle)
{
    if (lbl_title)
        lbl_title->SetText(FText::FromString(InTitle));
}

FString UConfirmDlg::GetMessage() const
{
    return lbl_message ? lbl_message->GetText().ToString() : FString();
}

void UConfirmDlg::SetMessage(const FString& InMessage)
{
    if (lbl_message)
        lbl_message->SetText(FText::FromString(InMessage));
}

void UConfirmDlg::OnApplyClicked()
{
    // Classic behavior: manager->HideConfirmDlg(); then parent_control user event.
    if (manager)
    {
        // Keep this callsite; implement in your MenuScreen port.
        // manager->HideConfirmDlg();
    }

    OnConfirmed.Broadcast();
}

void UConfirmDlg::OnCancelClicked()
{
    if (manager)
    {
        // manager->HideConfirmDlg();
    }

    OnCanceled.Broadcast();
}

// --------------------------------------------------------------------
// UBaseScreen overrides
// --------------------------------------------------------------------

void UConfirmDlg::BindFormWidgets()
{
    // Map FORM ids to widgets for legacy default application
    BindButton(1, btn_apply);
    BindButton(2, btn_cancel);
    BindLabel(100, lbl_title);
    BindLabel(101, lbl_message);

    // Ensure centralized Enter/Escape uses these:
    ApplyButton = btn_apply;
    CancelButton = btn_cancel;
}

FString UConfirmDlg::GetLegacyFormText() const
{
    // Original ConfirmDlg.frm content, preserved verbatim (minus file header comments).
    // NOTE: Your parser accepts optional commas; keep format stable.
    return TEXT(R"FORM(
form: {
   rect:       (0,0,400,280)
   back_color: (0,0,0)
   fore_color: (255,255,255)
   font:       Limerick12

   texture:    "Message.pcx"
   margins:    (50,40,48,40)

   layout: {
      x_mins:     (20, 40, 100, 100, 20),
      x_weights:  ( 0,  0,   1,   1,  0),

      y_mins:     (44,  30, 30, 80, 35),
      y_weights:  ( 0,   0,  0,  1,  0)
   }

   defctrl: {
      fore_color:    (255, 255, 255)
      font:          Limerick12
      transparent:   true
   }

   ctrl: {
      id:            100
      type:          label
      text:          "Are You Sure?"
      align:         center
      cells:         (1,1,3,1)
      font:          Limerick18
   }

   ctrl: {
      id:            101
      type:          label
      text:          "Are you sure you want to take this action?"
      align:         left
      font:          Verdana
      cells:         (1,2,3,1)
   }

   defctrl: {
      align:            left
      font:             Limerick12
      fore_color:       (0,0,0)
      standard_image:   Button17_0
      activated_image:  Button17_1
      transition_image: Button17_2
      bevel_width:      6
      margins:          (3,18,0,0)
      cell_insets:      (10,0,0,16)
      transparent:      false
   }

   ctrl: {
      id:            1,
      type:          button,
      cells:         (2,4,1,1)
      text:          "OK"
   }

   ctrl: {
      id:            2,
      type:          button,
      cells:         (3,4,1,1)
      text:          "Cancel"
   }
}
)FORM");
}
