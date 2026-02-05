#include "KeyDlg.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

#include "StarshatterKeyboardSettings.h"
#include "StarshatterKeyboardSubsystem.h"

#include "OptionsScreen.h"

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

    // BaseScreen routing (if these exist in your BaseScreen):
    ApplyButton = ApplyBtn;
    CancelButton = CancelBtn;

    if (ClearButton)
    {
        ClearButton->OnClicked.RemoveAll(this);
        ClearButton->OnClicked.AddDynamic(this, &UKeyDlg::OnClearClicked);
    }

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

void UKeyDlg::NativeConstruct()
{
    Super::NativeConstruct();

    bKeyClear = false;
    bCapturing = true;
    PendingKey = FKey();

    RefreshDisplayFromSettings();

    if (NewKeyText)
        NewKeyText->SetText(FText::GetEmpty());

    SetKeyboardFocus();
}

void UKeyDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    (void)MyGeometry;
    (void)InDeltaTime;

    // UE-only: no polling required. Keep ExecFrame for legacy signature parity.
    ExecFrame();
}

FReply UKeyDlg::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    (void)InGeometry;

    if (!bCapturing)
        return Super::NativeOnKeyDown(InGeometry, InKeyEvent);

    const FKey PressedKey = InKeyEvent.GetKey();

    if (PressedKey == EKeys::Escape)
    {
        OnCancelClicked();
        return FReply::Handled();
    }

    if (!PressedKey.IsValid())
        return FReply::Handled();

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

void UKeyDlg::ExecFrame()
{
    // UE-only: do nothing.
}

void UKeyDlg::SetManager(UOptionsScreen* InManager)
{
    Manager = InManager;
}

void UKeyDlg::SetEditingAction(EStarshatterInputAction InAction)
{
    EditingAction = InAction;
    KeyIndex = (int32)InAction;

    PendingKey = FKey();
    bKeyClear = false;
    bCapturing = true;

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

    PendingKey = FKey();
    bKeyClear = false;
    bCapturing = true;

    RefreshDisplayFromSettings();
    SetKeyboardFocus();
}

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

    SetVisibility(ESlateVisibility::Collapsed);

    if (Manager)
    {
        Manager->SetVisibility(ESlateVisibility::Visible);
        Manager->ShowOptDlg();
        Manager->SetKeyboardFocus();
    }
}

void UKeyDlg::OnCancelClicked()
{
    SetVisibility(ESlateVisibility::Collapsed);

    if (Manager)
    {
        Manager->SetVisibility(ESlateVisibility::Visible);
        Manager->ShowOptDlg();
        Manager->SetKeyboardFocus();
    }
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
    Settings->Sanitize();
    Settings->Save();

    if (UStarshatterKeyboardSubsystem* KBSS = UStarshatterKeyboardSubsystem::Get(this))
        KBSS->ApplySettingsToRuntime(this);

    CurrentKey = PendingKey;
    PendingKey = FKey();
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
    Settings->Sanitize();
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

    return FString("ACTION");
}

FString UKeyDlg::KeyToDisplayString(const FKey& Key)
{
    if (!Key.IsValid())
        return FString("UNBOUND");

    return Key.GetDisplayName().ToString();
}

void UKeyDlg::SetTextBlock(UTextBlock* Block, const FString& Text)
{
    if (!Block)
        return;

    Block->SetText(FText::FromString(Text));
}
