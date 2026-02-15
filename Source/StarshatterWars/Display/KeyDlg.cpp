/*=============================================================================
    Project:        Starshatter Wars
    Studio:         Fractal Dev Studios
    Copyright:      (c) 2025-2026.
    All Rights Reserved.

    SUBSYSTEM:      StarshatterWars (Unreal Engine)
    FILE:           KeyDlg.cpp
    AUTHOR:         Carlos Bott
=============================================================================*/

#include "KeyDlg.h"

// UMG
#include "Components/Button.h"
#include "Components/TextBlock.h"

// Input
#include "Input/Reply.h"
#include "InputCoreTypes.h"

// Router
#include "OptionsScreen.h"

// Model + runtime
#include "StarshatterKeyboardSettings.h"
#include "StarshatterKeyboardSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogKeyDlg, Log, All);

UKeyDlg::UKeyDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SetDialogInputEnabled(true);
    bIsFocusable = true;
}

void UKeyDlg::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // BaseScreen Enter/Escape routing (if BaseScreen uses these):
    ApplyButton = ApplyBtn;
    CancelButton = CancelBtn;

    BindDelegatesOnce();
}

void UKeyDlg::NativeConstruct()
{
    Super::NativeConstruct();

    // Safe to call; guarded:
    BindDelegatesOnce();

    bKeyClear = false;

    BeginCapture();
    RefreshDisplayFromSettings();

    if (NewKeyText)
        NewKeyText->SetText(FText::GetEmpty());

    SetKeyboardFocus();
}

void UKeyDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    (void)MyGeometry;
    ExecFrame((double)InDeltaTime);
}

void UKeyDlg::ExecFrame(double /*DeltaTime*/)
{
    // UE-only: no polling.
}

void UKeyDlg::BindDelegatesOnce()
{
    if (bDelegatesBound)
        return;

    if (ClearButton)
        ClearButton->OnClicked.AddUniqueDynamic(this, &UKeyDlg::OnClearClicked);

    if (ApplyBtn)
        ApplyBtn->OnClicked.AddUniqueDynamic(this, &UKeyDlg::OnApplyClicked);

    if (CancelBtn)
        CancelBtn->OnClicked.AddUniqueDynamic(this, &UKeyDlg::OnCancelClicked);

    bDelegatesBound = true;
}

FReply UKeyDlg::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    (void)InGeometry;

    const FKey PressedKey = InKeyEvent.GetKey();

    // ESC always returns (Cancel)
    if (PressedKey == EKeys::Escape || PressedKey == EKeys::Virtual_Back)
    {
        OnCancelClicked();
        return FReply::Handled();
    }

    if (!bCapturing)
        return Super::NativeOnKeyDown(InGeometry, InKeyEvent);

    if (!PressedKey.IsValid())
        return FReply::Handled();

    // Ignore pure modifiers
    if (PressedKey == EKeys::LeftShift || PressedKey == EKeys::RightShift ||
        PressedKey == EKeys::LeftControl || PressedKey == EKeys::RightControl ||
        PressedKey == EKeys::LeftAlt || PressedKey == EKeys::RightAlt ||
        PressedKey == EKeys::LeftCommand || PressedKey == EKeys::RightCommand)
    {
        return FReply::Handled();
    }

    PendingKey = PressedKey;
    bKeyClear = false;
    bCapturing = false;

    if (NewKeyText)
        SetTextBlock(NewKeyText, KeyToDisplayString(PendingKey));

    return FReply::Handled();
}

void UKeyDlg::SetOptionsManager(UOptionsScreen* InManager)
{
    OptionsManager = InManager;
}

void UKeyDlg::BeginCapture()
{
    PendingKey = FKey();
    bKeyClear = false;
    bCapturing = true;

    if (NewKeyText)
        NewKeyText->SetText(FText::GetEmpty());
}

void UKeyDlg::SetEditingAction(EStarshatterInputAction InAction)
{
    EditingAction = InAction;
    KeyIndex = (int32)InAction;

    BeginCapture();
    RefreshDisplayFromSettings();
    SetKeyboardFocus();
}

void UKeyDlg::SetKeyMapIndex(int i)
{
    KeyIndex = i;

    const int64 Value = (int64)i;
    const UEnum* Enum = StaticEnum<EStarshatterInputAction>();
    if (Enum && Enum->IsValidEnumValue(Value))
        EditingAction = (EStarshatterInputAction)i;
    else
        EditingAction = EStarshatterInputAction::Pause;

    BeginCapture();
    RefreshDisplayFromSettings();
    SetKeyboardFocus();
}

void UKeyDlg::HandleAccept() { OnApplyClicked(); }
void UKeyDlg::HandleCancel() { OnCancelClicked(); }

void UKeyDlg::OnClearClicked()
{
    bKeyClear = true;
    PendingKey = FKey();
    bCapturing = false;

    if (NewKeyText)
        NewKeyText->SetText(FText::GetEmpty());
}

void UKeyDlg::OnApplyClicked()
{
    if (bKeyClear)
    {
        ClearFromSettings();
    }
    else if (PendingKey.IsValid())
    {
        CommitPendingToSettings();
    }

    // OptionsScreen owns routing; we just ask to return:
    if (OptionsManager)
        OptionsManager->ReturnFromKeyDlg();
    else
        SetVisibility(ESlateVisibility::Collapsed);
}

void UKeyDlg::OnCancelClicked()
{
    // No commits; just return
    if (OptionsManager)
        OptionsManager->ReturnFromKeyDlg();
    else
        SetVisibility(ESlateVisibility::Collapsed);
}

void UKeyDlg::CommitPendingToSettings()
{
    UStarshatterKeyboardSettings* Settings = UStarshatterKeyboardSettings::Get();
    if (!Settings)
        return;

    Settings->Load();

    FStarshatterKeyboardConfig Cfg = Settings->GetKeyboardConfig();
    Cfg.RemappedKeys.Add(EditingAction, PendingKey);

    Settings->SetKeyboardConfig(Cfg);
    Settings->Save();

    if (UStarshatterKeyboardSubsystem* KBSS = UStarshatterKeyboardSubsystem::Get(this))
        KBSS->ApplySettingsToRuntime(this);

    CurrentKey = PendingKey;
    PendingKey = FKey();
    bKeyClear = false;
}

void UKeyDlg::ClearFromSettings()
{
    UStarshatterKeyboardSettings* Settings = UStarshatterKeyboardSettings::Get();
    if (!Settings)
        return;

    Settings->Load();

    FStarshatterKeyboardConfig Cfg = Settings->GetKeyboardConfig();
    Cfg.RemappedKeys.Remove(EditingAction);

    Settings->SetKeyboardConfig(Cfg);
    Settings->Save();

    if (UStarshatterKeyboardSubsystem* KBSS = UStarshatterKeyboardSubsystem::Get(this))
        KBSS->ApplySettingsToRuntime(this);

    CurrentKey = FKey();
    PendingKey = FKey();
    bKeyClear = false;
}

void UKeyDlg::RefreshDisplayFromSettings()
{
    UStarshatterKeyboardSettings* Settings = UStarshatterKeyboardSettings::Get();
    if (!Settings)
        return;

    Settings->Load();

    const FStarshatterKeyboardConfig& Cfg = Settings->GetKeyboardConfig();
    if (const FKey* Found = Cfg.RemappedKeys.Find(EditingAction))
        CurrentKey = *Found;
    else
        CurrentKey = FKey();

    if (CommandText)
        SetTextBlock(CommandText, ActionToDisplayString(EditingAction));

    if (CurrentKeyText)
        SetTextBlock(CurrentKeyText, KeyToDisplayString(CurrentKey));

    if (NewKeyText)
        NewKeyText->SetText(FText::GetEmpty());
}

FString UKeyDlg::ActionToDisplayString(EStarshatterInputAction Action)
{
    const UEnum* Enum = StaticEnum<EStarshatterInputAction>();
    if (Enum)
        return Enum->GetDisplayNameTextByValue((int64)Action).ToString();

    return TEXT("ACTION");
}

FString UKeyDlg::KeyToDisplayString(const FKey& Key)
{
    if (!Key.IsValid())
        return TEXT("UNBOUND");

    return Key.GetDisplayName().ToString();
}

void UKeyDlg::SetTextBlock(UTextBlock* Block, const FString& Text)
{
    if (Block)
        Block->SetText(FText::FromString(Text));
}
