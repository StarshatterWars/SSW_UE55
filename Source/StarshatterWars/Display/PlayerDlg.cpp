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
    SelectedPlayer = nullptr;
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
    // Unbind to be safe
    if (lst_roster)
        lst_roster->OnItemSelectionChanged().RemoveAll(this);

    if (btn_add) btn_add->OnClicked.RemoveAll(this);
    if (btn_del) btn_del->OnClicked.RemoveAll(this);

    if (BtnSave) BtnSave->OnClicked.RemoveAll(this);
    if (BtnCancel) BtnCancel->OnClicked.RemoveAll(this);

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
    if (BtnSave)
    {
        BtnSave->OnClicked.RemoveAll(this);
        BtnSave->OnClicked.AddDynamic(this, &UPlayerDlg::OnApply);
    }
    else if (ApplyButton)
    {
        ApplyButton->OnClicked.RemoveAll(this);
        ApplyButton->OnClicked.AddDynamic(this, &UPlayerDlg::OnApply);
    }

    if (BtnCancel)
    {
        BtnCancel->OnClicked.RemoveAll(this);
        BtnCancel->OnClicked.AddDynamic(this, &UPlayerDlg::OnCancel);
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

    // Preserve selection by PlayerId if possible:
    const int32 PrevId = (selected_item && selected_item->GetPlayer())
        ? selected_item->GetPlayer()->GetIdentity()
        : 0;

    lst_roster->ClearListItems();
    selected_item = nullptr;
    SelectedPlayer = nullptr;

    List<PlayerCharacter>& roster = PlayerCharacter::GetRoster();
    if (roster.size() < 1)
    {
        ShowPlayer();
        return;
    }

    UPlayerRosterItem* FirstItem = nullptr;
    UPlayerRosterItem* MatchItem = nullptr;

    for (int i = 0; i < roster.size(); ++i)
    {
        PlayerCharacter* player = roster[i];
        if (!player)
            continue;

        UPlayerRosterItem* item = NewObject<UPlayerRosterItem>(this);
        item->Initialize(player);

        if (!FirstItem)
            FirstItem = item;

        if (PrevId != 0 && player->GetIdentity() == PrevId)
            MatchItem = item;

        lst_roster->AddItem(item);
    }

    // Select preserved item if present; else first:
    selected_item = MatchItem ? MatchItem : FirstItem;

    if (selected_item)
    {
        lst_roster->SetSelectedItem(selected_item);
        SelectedPlayer = selected_item->GetPlayer();

        // Keep legacy selection in sync if you want:
        if (SelectedPlayer)
            PlayerCharacter::SetCurrentPlayer(SelectedPlayer);
    }

    ShowPlayer();
}

void UPlayerDlg::RefreshRoster()
{
    // Safe approach: rebuild
    BuildRoster();
}

void UPlayerDlg::OnRosterSelectionChanged(UObject* SelectedItem)
{
    UPlayerRosterItem* Item = Cast<UPlayerRosterItem>(SelectedItem);
    PlayerCharacter* player = Item ? Item->GetPlayer() : nullptr;

    // Update our selection pointers (IMPORTANT):
    selected_item = Item;
    SelectedPlayer = player;

    if (player)
        PlayerCharacter::SetCurrentPlayer(player);

    // IMPORTANT: do NOT call UpdatePlayer() here — selection change should not overwrite data.
    // Just show selected record:
    ShowPlayer();
}

PlayerCharacter* UPlayerDlg::GetSelectedPlayer() const
{
    if (selected_item)
        return selected_item->GetPlayer();

    return nullptr;
}

// +--------------------------------------------------------------------+
// UI <-> Model
// +--------------------------------------------------------------------+

void UPlayerDlg::UpdatePlayer()
{
    PlayerCharacter* player = GetSelectedPlayer();
    if (!player)
        return;

    if (edt_name)
    {
        const FString NewName = edt_name->GetText().ToString();
        if (!NewName.IsEmpty())
            player->SetName(NewName);
    }
}

void UPlayerDlg::ShowPlayer()
{
    PlayerCharacter* player = GetSelectedPlayer();

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
        edt_name->SetText(FText::FromString(player->Name()));   // FIXED

    if (lbl_rank)
        lbl_rank->SetText(FText::AsNumber(player->GetRank()));

    if (lbl_flighttime)
        lbl_flighttime->SetText(FText::FromString(FormatTimeHMS(player->FlightTime())));

    if (lbl_createdate)
        lbl_createdate->SetText(FText::FromString(FormatDateFromUnixSeconds((int64)player->CreateDate())));

    if (lbl_kills)
        lbl_kills->SetText(FText::AsNumber(player->Kills()));

    if (lbl_deaths)
        lbl_deaths->SetText(FText::AsNumber(player->Deaths()));

    if (lbl_missions)
        lbl_missions->SetText(FText::AsNumber(player->Missions()));
}

// +--------------------------------------------------------------------+
// Actions
// +--------------------------------------------------------------------+

void UPlayerDlg::OnAdd()
{
    // Commit current edits first:
    UpdatePlayer();

    PlayerCharacter* new_player = PlayerCharacter::CreateDefault();
    if (!new_player)
        return;

    PlayerCharacter::AddToRoster(new_player);
    PlayerCharacter::SetCurrentPlayer(new_player);
    PlayerCharacter::Save();
    
    // Force selection to new player by setting selected_item after rebuild:
    selected_item = nullptr;
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
    SelectedPlayer = nullptr;
    RefreshRoster();
}

void UPlayerDlg::OnApply()
{
    UpdatePlayer();
    PlayerCharacter::Save();

    if (manager)
        manager->ShowMenuDlg();
    else
        HideDlg();
}

void UPlayerDlg::OnCancel()
{
    // Discard UI edits by re-showing current model:
    ShowPlayer();

    if (manager)
        manager->ShowMenuDlg();
    else
        HideDlg();
}

// +--------------------------------------------------------------------+
// Formatting helpers
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
    // If CreateDate() is not Unix seconds in your legacy model, map it here.
    if (UnixSeconds <= 0)
        return TEXT("");

    const FDateTime DT = FDateTime::FromUnixTimestamp(UnixSeconds);
    return DT.ToString(TEXT("%Y-%m-%d"));
}
