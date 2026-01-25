/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         ConfirmDlg.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    General-purpose confirmation dialog (Unreal UUserWidget)

    Legacy behavior mapping:
    - Enter => Apply
    - Escape => Cancel
    - On Apply: Hide confirm dlg, then notify parent control (legacy ClientEvent(EID_USER_1))
      In UE: if parent_control implements Blueprint event "OnConfirmAccepted", we call it.
*/

#include "GameStructs.h"

#include "ConfirmDlg.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Input/Reply.h"
#include "InputCoreTypes.h"

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
        UE_LOG(LogTemp, Warning, TEXT("ConfirmDlg: Manager does not implement '%s'."), fnName);
    }
}

static void NotifyParentAccepted(UWidget* parent)
{
    if (!parent) return;

    // Preferred: BlueprintImplementableEvent on the parent widget named "OnConfirmAccepted"
    // (no params). This mirrors the old EID_USER_1 callback.
    const FName fname(TEXT("OnConfirmAccepted"));
    UFunction* fn = parent->FindFunction(fname);
    if (fn)
    {
        parent->ProcessEvent(fn, nullptr);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ConfirmDlg: Parent control does not implement OnConfirmAccepted()."));
    }
}

// +--------------------------------------------------------------------+

UConfirmDlg::UConfirmDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UConfirmDlg::NativeConstruct()
{
    Super::NativeConstruct();

    if (ApplyBtn)
    {
        ApplyBtn->OnClicked.AddDynamic(this, &UConfirmDlg::OnApplyClicked);
    }

    if (CancelBtn)
    {
        CancelBtn->OnClicked.AddDynamic(this, &UConfirmDlg::OnCancelClicked);
    }

    // Push cached values into widgets (in case set before construct):
    if (lbl_title)   lbl_title->SetText(title_text);
    if (lbl_message) lbl_message->SetText(message_text);

    // Legacy: play confirm sound when first shown. We do not have the old Button::PlaySound,
    // so we log for now (you can wire to your UI sound system later).
    UE_LOG(LogTemp, Log, TEXT("ConfirmDlg: Opened."));

    // Ensure we can receive Enter/Escape:
    SetKeyboardFocus();
}

void UConfirmDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
}

FReply UConfirmDlg::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    const FKey key = InKeyEvent.GetKey();

    if (key == EKeys::Enter || key == EKeys::Virtual_Accept)
    {
        Apply();
        return FReply::Handled();
    }

    if (key == EKeys::Escape || key == EKeys::Virtual_Back)
    {
        Cancel();
        return FReply::Handled();
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

// +--------------------------------------------------------------------+
// Title / Message setters:

void UConfirmDlg::SetTitleText(const FText& t)
{
    title_text = t;

    if (lbl_title)
    {
        lbl_title->SetText(title_text);
    }
}

void UConfirmDlg::SetMessageText(const FText& m)
{
    message_text = m;

    if (lbl_message)
    {
        lbl_message->SetText(message_text);
    }
}

// +--------------------------------------------------------------------+
// Button handlers:

void UConfirmDlg::OnApplyClicked()
{
    Apply();
}

void UConfirmDlg::OnCancelClicked()
{
    Cancel();
}

// +--------------------------------------------------------------------+
// Operations:

void UConfirmDlg::Apply()
{
    // Legacy: manager->HideConfirmDlg();
    CallManagerByName(manager, TEXT("HideConfirmDlg"));

    // Legacy: parent_control->ClientEvent(EID_USER_1);
    NotifyParentAccepted(parent_control);
}

void UConfirmDlg::Cancel()
{
    // Legacy: manager->HideConfirmDlg();
    CallManagerByName(manager, TEXT("HideConfirmDlg"));
}
