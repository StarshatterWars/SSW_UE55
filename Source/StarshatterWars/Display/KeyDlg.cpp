/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         KeyDlg.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Key Binding dialog (legacy KeyDlg) implementation for Unreal UMG.

    UPDATED ROUTING:
    - Returns to ControlOptionsDlg (Manager) on apply/cancel.
*/

#include "KeyDlg.h"

// Unreal
#include "Logging/LogMacros.h"

// UMG
#include "Components/Button.h"
#include "Components/TextBlock.h"

// Starshatter
#include "KeyMap.h"
#include "Starshatter.h"
#include "Joystick.h"

// UPDATED:
#include "ControlOptionsDlg.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <Windows.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogKeyDlg, Log, All);

// --------------------------------------------------------------------

UKeyDlg::UKeyDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

// --------------------------------------------------------------------

void UKeyDlg::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (ClearButton)
    {
        ClearButton->OnClicked.RemoveAll(this);
        ClearButton->OnClicked.AddDynamic(this, &UKeyDlg::OnClearClicked);
    }

    // NOTE: ApplyButton / CancelButton come from UBaseScreen:
    if (ApplyButton)
    {
        ApplyButton->OnClicked.RemoveAll(this);
        ApplyButton->OnClicked.AddDynamic(this, &UKeyDlg::OnApplyClicked);
    }

    if (CancelButton)
    {
        CancelButton->OnClicked.RemoveAll(this);
        CancelButton->OnClicked.AddDynamic(this, &UKeyDlg::OnCancelClicked);
    }
}

// --------------------------------------------------------------------

void UKeyDlg::NativeConstruct()
{
    Super::NativeConstruct();

    // Mirror legacy Show():
    RefreshDisplayFromCurrentBinding();

    bKeyClear = false;
    if (NewKeyText)
        NewKeyText->SetText(FText::GetEmpty());
}

// --------------------------------------------------------------------

void UKeyDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    ExecFrame();
}

// --------------------------------------------------------------------

void UKeyDlg::SetManager(UControlOptionsDlg* InManager)
{
    Manager = InManager;
}

// --------------------------------------------------------------------

void UKeyDlg::SetKeyMapIndex(int i)
{
    KeyIndex = i;
    KeyKey = 0;
    KeyShift = 0;
    KeyJoy = 0;
    bKeyClear = false;

    RefreshDisplayFromCurrentBinding();

    if (NewKeyText)
        NewKeyText->SetText(FText::GetEmpty());
}

// --------------------------------------------------------------------
// Legacy parity (ExecFrame)
// --------------------------------------------------------------------

void UKeyDlg::ExecFrame()
{
    int key = 0;
    int shift = 0;
    int joy = 0;

    Joystick* joystick = Joystick::GetInstance();
    if (joystick)
        joystick->Acquire();

    for (int i = 0; i < 256; i++)
    {
        const int vk = KeyMap::GetMappableVKey(i);

        if (vk >= KEY_JOY_1 && vk <= KEY_JOY_16)
        {
            if (joystick && joystick->KeyDown(vk))
                joy = vk;
        }
        else if (vk >= KEY_POV_0_UP && vk <= KEY_POV_3_RIGHT)
        {
            if (joystick && joystick->KeyDown(vk))
                joy = vk;
        }
        else
        {
#if PLATFORM_WINDOWS
            if (GetAsyncKeyState(vk))
            {
                if (vk == VK_SHIFT || vk == VK_MENU)
                    shift = vk;
                else
                    key = vk;
            }
#endif
        }
    }

    if (key)
    {
        KeyKey = key;
        KeyShift = shift;
        KeyJoy = joy; // keep any joy detected this frame

        if (NewKeyText)
        {
            const char* desc = KeyMap::DescribeKey(key, shift, joy);
            SetTextBlock(NewKeyText, desc ? desc : "");
        }
    }
    else if (joy)
    {
        KeyJoy = joy;

        if (NewKeyText)
        {
            const char* desc = KeyMap::DescribeKey(0, 0, joy);
            SetTextBlock(NewKeyText, desc ? desc : "");
        }
    }
}

// --------------------------------------------------------------------

void UKeyDlg::OnClearClicked()
{
    bKeyClear = true;
    KeyKey = 0;
    KeyShift = 0;
    KeyJoy = 0;

    if (NewKeyText)
        NewKeyText->SetText(FText::GetEmpty());
}

// --------------------------------------------------------------------

void UKeyDlg::OnApplyClicked()
{
    Starshatter* stars = Starshatter::GetInstance();

    if (stars)
    {
        KeyMap& keymap = stars->GetKeyMap();
        KeyMapEntry* map = keymap.GetKeyMap(KeyIndex);

        if (map)
        {
            if (bKeyClear)
            {
                map->key = 0;
                map->alt = 0;
                map->joy = 0;
            }

            if (KeyKey)
            {
                map->key = KeyKey;
                map->alt = KeyShift;
            }

            if (KeyJoy)
            {
                map->joy = KeyJoy;
            }
        }
    }
    else
    {
        UE_LOG(LogKeyDlg, Warning, TEXT("Starshatter instance not available."));
    }

    // Hide this dialog and return to Controls page:
    SetVisibility(ESlateVisibility::Collapsed);

    if (Manager)
    {
        Manager->SetVisibility(ESlateVisibility::Visible);
        Manager->Show();          // refresh list / selection text
        Manager->SetKeyboardFocus();
    }
}

// --------------------------------------------------------------------

void UKeyDlg::OnCancelClicked()
{
    // Hide this dialog and return to Controls page:
    SetVisibility(ESlateVisibility::Collapsed);

    if (Manager)
    {
        Manager->SetVisibility(ESlateVisibility::Visible);
        Manager->Show();
        Manager->SetKeyboardFocus();
    }
}

// --------------------------------------------------------------------
// Helpers
// --------------------------------------------------------------------

void UKeyDlg::RefreshDisplayFromCurrentBinding()
{
    Starshatter* stars = Starshatter::GetInstance();
    if (!stars)
        return;

    KeyMap& keymap = stars->GetKeyMap();

    if (CommandText)
        SetTextBlock(CommandText, keymap.DescribeAction(KeyIndex));

    if (CurrentKeyText)
        SetTextBlock(CurrentKeyText, keymap.DescribeKey(KeyIndex));
}

void UKeyDlg::SetTextBlock(UTextBlock* Block, const char* AnsiText)
{
    if (!Block)
        return;

    if (!AnsiText)
        AnsiText = "";

    Block->SetText(FText::FromString(UTF8_TO_TCHAR(AnsiText)));
}

void UKeyDlg::SetTextBlock(UTextBlock* Block, const FString& Text)
{
    if (!Block)
        return;

    Block->SetText(FText::FromString(Text));
}
