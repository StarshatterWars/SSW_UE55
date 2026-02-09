/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.
    All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         PlayerDlg.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Player dialog (UE port) implementation
*/

#include "PlayerDlg.h"

#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/Image.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"

#include "Input/Reply.h"
#include "InputCoreTypes.h"

#include "PlayerRosterItem.h"
#include "PlayerCharacter.h"
#include "MenuScreen.h"

// +--------------------------------------------------------------------+

UPlayerDlg::UPlayerDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // defaults only
}

void UPlayerDlg::InitializeDlg(UMenuScreen* InManager)
{
    manager = InManager;
}

void UPlayerDlg::NativeConstruct()
{
    Super::NativeConstruct();

    RegisterControls();
    BuildRoster();
    ShowPlayer();
}

void UPlayerDlg::NativeDestruct()
{
    // Unbind to be safe (optional)
    if (lst_roster)
        lst_roster->OnItemSelectionChanged().RemoveAll(this);

    if (btn_add) btn_add->OnClicked.RemoveAll(this);
    if (btn_del) btn_del->OnClicked.RemoveAll(this);

    if (apply) apply->OnClicked.RemoveAll(this);
    if (cancel) cancel->OnClicked.RemoveAll(this);

    if (ApplyButton) ApplyButton->OnClicked.RemoveAll(this);
    if (CancelButton) CancelButton->OnClicked.RemoveAll(this);

    Super::NativeDestruct();
}

FReply UPlayerDlg::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
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

void UPlayerDlg::ShowDlg()
{
    SetVisibility(ESlateVisibility::Visible);
    BuildRoster();
    ShowPlayer();
}

void UPlayerDlg::HideDlg()
{
    SetVisibility(ESlateVisibility::Collapsed);
}

// +--------------------------------------------------------------------+
// Controls
// +--------------------------------------------------------------------+

void UPlayerDlg::RegisterControls()
{
    // Roster select:
    if (lst_roster)
    {
        lst_roster->OnItemSelectionChanged().RemoveAll(this);
        lst_roster->OnItemSelectionChanged().AddUObject(this, &UPlayerDlg::OnRosterSelectionChanged);
    }

    // Add/Delete:
    if (btn_add)
    {
        btn_add->OnClicked.RemoveAll(this);
        btn_add->OnClicked.AddDynamic(this, &UPlayerDlg::OnAdd);
    }

    if (btn_del)
    {
        btn_del->OnClicked.RemoveAll(this);
        btn_del->OnClicked.AddDynamic(this, &UPlayerDlg::OnDel);
    }

    // Apply/Cancel (prefer local; fall back to BaseScreen buttons):
    if (apply)
    {
        apply->OnClicked.RemoveAll(this);
        apply->OnClicked.AddDynamic(this, &UPlayerDlg::OnApply);
    }
    else if (ApplyButton)
    {
        ApplyButton->OnClicked.RemoveAll(this);
        ApplyButton->OnClicked.AddDynamic(this, &UPlayerDlg::OnApply);
    }

    if (cancel)
    {
        cancel->OnClicked.RemoveAll(this);
        cancel->OnClicked.AddDynamic(this, &UPlayerDlg::OnCancel);
    }
    else if (CancelButton)
    {
        CancelButton->OnClicked.RemoveAll(this);
        CancelButton->OnClicked.AddDynamic(this, &UPlayerDlg::OnCancel);
    }
}

// +--------------------------------------------------------------------+
// Roster list
// +--------------------------------------------------------------------+

void UPlayerDlg::BuildRoster()
{
    if (!lst_roster)
        return;

    lst_roster->ClearListItems();

    List<PlayerCharacter>& roster = PlayerCharacter::GetRoster();
    if (roster.size() < 1)
    {
        selected_item = nullptr;
        ShowPlayer();
        return;
    }

    for (int i = 0; i < roster.size(); ++i)
    {
        PlayerCharacter* player = roster[i];
        if (!player)
            continue;

        UPlayerRosterItem* item = NewObject<UPlayerRosterItem>(this);
        item->Initialize(player);

        lst_roster->AddItem(item);

        // Default selection: first item if none selected:
        if (!selected_item)
            selected_item = item;
    }

    if (selected_item)
        lst_roster->SetSelectedItem(selected_item);
}

void UPlayerDlg::RefreshRoster()
{
    // Simple approach (safe): rebuild.
    BuildRoster();
}

void UPlayerDlg::OnRosterSelectionChanged(UObject* SelectedItem)
{
    // If you are using a UObject item wrapper:
    UPlayerRosterItem* Item = Cast<UPlayerRosterItem>(SelectedItem);
    PlayerCharacter* player = Item ? Item->GetPlayer() : nullptr;

    // Store selection however you do it:
    SelectedPlayer = player;

    UpdatePlayer();
    ShowPlayer();
}

PlayerCharacter* UPlayerDlg::GetSelectedPlayer() const
{
    if (!selected_item)
        return nullptr;

    return selected_item->GetPlayer();
}

// +--------------------------------------------------------------------+
// UI <-> Model
// +--------------------------------------------------------------------+

void UPlayerDlg::UpdatePlayer()
{
    PlayerCharacter* player = GetSelectedPlayer();
    if (!player)
        return;

    // Name:
    if (edt_name)
    {
        const FString NewName = edt_name->GetText().ToString();
        if (!NewName.IsEmpty())
            player->SetName(TCHAR_TO_ANSI(*NewName));
    }

    // Add more fields here if you expose them in UMG.
}

void UPlayerDlg::ShowPlayer()
{
    PlayerCharacter* player = GetSelectedPlayer();

    // Clear UI if no selection:
    if (!player)
    {
        if (edt_name)        edt_name->SetText(FText::GetEmpty());
        if (lbl_rank)        lbl_rank->SetText(FText::GetEmpty());
        if (lbl_flighttime)  lbl_flighttime->SetText(FText::GetEmpty());
        if (lbl_createdate)  lbl_createdate->SetText(FText::GetEmpty());
        if (lbl_kills)       lbl_kills->SetText(FText::GetEmpty());
        if (lbl_deaths)      lbl_deaths->SetText(FText::GetEmpty());
        if (lbl_missions)    lbl_missions->SetText(FText::GetEmpty());
        return;
    }

    if (edt_name)
        edt_name->SetText(FText::FromString(ANSI_TO_TCHAR(player->Name())));

    if (lbl_rank)
    {
        const int32 RankVal = (int32)player->   GetRank();
        lbl_rank->SetText(FText::AsNumber(RankVal));
    }

    if (lbl_flighttime)
        lbl_flighttime->SetText(FText::FromString(FormatTimeHMS(player->FlightTime())));

    if (lbl_createdate)
        lbl_createdate->SetText(FText::FromString(FormatDateFromUnixSeconds((int64)player->CreateDate())));

    if (lbl_kills)
        lbl_kills->SetText(FText::AsNumber((int32)player->Kills()));

    if (lbl_deaths)
        lbl_deaths->SetText(FText::AsNumber((int32)player->Deaths()));

    if (lbl_missions)
        lbl_missions->SetText(FText::AsNumber((int32)player->Missions()));

    // Rank/medal images: assign brushes here when your texture pipeline is ready.
    // img_rank->SetBrushFromTexture(...)
    // img_medal->SetBrushFromTexture(...)
}

// +--------------------------------------------------------------------+
// Actions
// +--------------------------------------------------------------------+

void UPlayerDlg::OnAdd()
{
    // Commit current edits:
    UpdatePlayer();

    PlayerCharacter* new_player = PlayerCharacter::CreateDefault();
    if (!new_player)
        return;

    PlayerCharacter::AddToRoster(new_player);
    PlayerCharacter::Save();

    RefreshRoster();
}

void UPlayerDlg::OnDel()
{
    PlayerCharacter* player = GetSelectedPlayer();
    if (!player)
        return;

    PlayerCharacter::RemoveFromRoster(player);
    PlayerCharacter::Save();

    selected_item = nullptr;
    RefreshRoster();
}

void UPlayerDlg::OnApply()
{
    UpdatePlayer();
    PlayerCharacter::Save();

    if (manager)
        manager->ShowMenuDlg(); // adjust to your actual API
    else
        HideDlg();
}

void UPlayerDlg::OnCancel()
{
    // Discard UI edits by reloading selected player state:
    ShowPlayer();

    if (manager)
        manager->ShowMenuDlg(); // adjust to your actual API
    else
        HideDlg();
}

// +--------------------------------------------------------------------+
// Formatting helpers (replaces missing FormatTime / FormatTimeString)
// +--------------------------------------------------------------------+

FString UPlayerDlg::FormatTimeHMS(double Seconds)
{
    if (Seconds < 0.0)
        Seconds = 0.0;

    const int64 TotalSeconds = (int64)Seconds;
    const int64 H = TotalSeconds / 3600;
    const int64 M = (TotalSeconds % 3600) / 60;
    const int64 S = (TotalSeconds % 60);

    return FString::Printf(TEXT("%lld:%02lld:%02lld"), H, M, S);
}

FString UPlayerDlg::FormatDateFromUnixSeconds(int64 UnixSeconds)
{
    // If legacy CreateDate() is not Unix seconds, replace this mapping here.
    if (UnixSeconds <= 0)
        return TEXT("");

    const FDateTime DT = FDateTime::FromUnixTimestamp(UnixSeconds);
    return DT.ToString(TEXT("%Y-%m-%d"));
}
