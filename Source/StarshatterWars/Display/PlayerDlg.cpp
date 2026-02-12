/*=============================================================================
    Project:        Starshatter Wars (nGenEx Unreal Port)
    Studio:         Fractal Dev Games
    Copyright:      (C) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:      Stars.exe
    FILE:           PlayerDlg.cpp
    AUTHOR:         Carlos Bott

    OVERVIEW
    ========
    Player dialog (UE port) implementation

    PORT NOTE
    =========
    Legacy UI depended on PlayerCharacter for:
      - roster list (GetRoster/AddToRoster/RemoveFromRoster)
      - persistence (Save)
      - profile fields (Name/Rank/Kills/Deaths/Missions/etc.)

    Unreal port replaces PlayerCharacter with:
      - UStarshatterPlayerSubsystem (persistence + authoritative state)
      - FS_PlayerGameInfo (profile model)

    CURRENT MODEL
    =============
    Single-profile save slot.
    ListView shows one row (the active profile).
=============================================================================*/

#include "PlayerDlg.h"

#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/Image.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"

#include "Input/Reply.h"
#include "InputCoreTypes.h"

#include "Kismet/GameplayStatics.h"

#include "PlayerRosterItem.h"
#include "MenuScreen.h"
#include "StarshatterPlayerSubsystem.h"

// +--------------------------------------------------------------------+
// Construction
// +--------------------------------------------------------------------+

UPlayerDlg::UPlayerDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Defaults only
}

void UPlayerDlg::InitializeDlg(UMenuScreen* InManager)
{
    manager = InManager;
}

// +--------------------------------------------------------------------+
// UUserWidget lifecycle
// +--------------------------------------------------------------------+

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

    if (apply)  apply->OnClicked.RemoveAll(this);
    if (cancel) cancel->OnClicked.RemoveAll(this);

    if (ApplyButton)  ApplyButton->OnClicked.RemoveAll(this);
    if (CancelButton) CancelButton->OnClicked.RemoveAll(this);

    Super::NativeDestruct();
}

// +--------------------------------------------------------------------+
// Input
// +--------------------------------------------------------------------+

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

// +--------------------------------------------------------------------+
// Dialog visibility
// +--------------------------------------------------------------------+

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
// Subsystem accessor (local helper)
// +--------------------------------------------------------------------+

UStarshatterPlayerSubsystem* UPlayerDlg::GetPlayerSubsystem() const
{
    return UStarshatterPlayerSubsystem::Get(GetWorld());
}

// +--------------------------------------------------------------------+
// Roster list (single-profile model)
// +--------------------------------------------------------------------+

void UPlayerDlg::BuildRoster()
{
    if (!lst_roster)
        return;

    lst_roster->ClearListItems();
    selected_item = nullptr;

    UStarshatterPlayerSubsystem* PlayerSS = GetPlayerSubsystem();
    if (!PlayerSS || !PlayerSS->HasLoaded())
    {
        ShowPlayer();
        return;
    }

    const FS_PlayerGameInfo Info = PlayerSS->GetPlayerInfoCopy();

    UPlayerRosterItem* Item = NewObject<UPlayerRosterItem>(this);
    Item->Initialize(Info);

    lst_roster->AddItem(Item);

    selected_item = Item;
    lst_roster->SetSelectedItem(Item);
}

void UPlayerDlg::RefreshRoster()
{
    BuildRoster();
}

void UPlayerDlg::OnRosterSelectionChanged(UObject* SelectedItem)
{
    selected_item = Cast<UPlayerRosterItem>(SelectedItem);

    // In single-profile model, selection is trivial, but we keep the method
    // for forward compatibility with multi-profile.
    UpdatePlayer();
    ShowPlayer();
}

const UPlayerRosterItem* UPlayerDlg::GetSelectedItem() const
{
    return selected_item;
}

// +--------------------------------------------------------------------+
// UI <-> Model
// +--------------------------------------------------------------------+

void UPlayerDlg::UpdatePlayer()
{
    UStarshatterPlayerSubsystem* PlayerSS = GetPlayerSubsystem();
    if (!PlayerSS || !PlayerSS->HasLoaded())
        return;

    FS_PlayerGameInfo& Info = PlayerSS->GetMutablePlayerInfo();

    // Name:
    if (edt_name)
    {
        const FString NewName = edt_name->GetText().ToString();
        if (!NewName.IsEmpty())
            Info.Name = NewName;
    }

    // Keep aliases consistent (if your gameplay writes PlayerKills etc.):
    Info.SyncRosterAliasesFromPlayerStats();

    PlayerSS->MarkDirty();
}

void UPlayerDlg::ShowPlayer()
{
    UStarshatterPlayerSubsystem* PlayerSS = GetPlayerSubsystem();

    if (!PlayerSS || !PlayerSS->HasLoaded())
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

    const FS_PlayerGameInfo Info = PlayerSS->GetPlayerInfoCopy();

    if (edt_name)
        edt_name->SetText(FText::FromString(Info.Name));

    if (lbl_rank)
        lbl_rank->SetText(FText::AsNumber(Info.Rank));

    if (lbl_flighttime)
        lbl_flighttime->SetText(FText::FromString(FormatTimeHMS((double)Info.FlightTime)));

    if (lbl_createdate)
        lbl_createdate->SetText(FText::FromString(FormatDateFromUnixSeconds((int64)Info.CreateTime)));

    if (lbl_kills)
        lbl_kills->SetText(FText::AsNumber(Info.Kills));

    if (lbl_deaths)
        lbl_deaths->SetText(FText::AsNumber(Info.Deaths));

    if (lbl_missions)
        lbl_missions->SetText(FText::AsNumber(Info.Missions));

    // Rank/medal images: assign brushes here when your texture pipeline is ready.
    // img_rank->SetBrushFromTexture(...)
    // img_medal->SetBrushFromTexture(...)
}

// +--------------------------------------------------------------------+
// Actions
// +--------------------------------------------------------------------+

void UPlayerDlg::OnAdd()
{
    UStarshatterPlayerSubsystem* PlayerSS = GetPlayerSubsystem();
    if (!PlayerSS)
        return;

    // Commit current edits:
    UpdatePlayer();

    // Single-profile model: reset to a new default profile
    FS_PlayerGameInfo& Info = PlayerSS->GetMutablePlayerInfo();
    Info = FS_PlayerGameInfo();

    // Minimal identity defaults:
    Info.Id = FMath::Rand(); // replace with your real id generator
    Info.CreateTime = FDateTime::UtcNow().ToUnixTimestamp();

    // Ensure alias fields reflect the current Player* stats:
    Info.SyncRosterAliasesFromPlayerStats();

    PlayerSS->MarkDirty();
    PlayerSS->SavePlayer(true);

    RefreshRoster();
    ShowPlayer();
}

void UPlayerDlg::OnDel()
{
    UStarshatterPlayerSubsystem* PlayerSS = GetPlayerSubsystem();
    if (!PlayerSS)
        return;

    // Delete the save slot:
    const FString SlotX = PlayerSS->GetSlotName();
    const int32   User = PlayerSS->GetUserIndex();

    UGameplayStatics::DeleteGameInSlot(SlotX, User);

    // Reset in-memory profile:
    FS_PlayerGameInfo& Info = PlayerSS->GetMutablePlayerInfo();
    Info = FS_PlayerGameInfo();
    Info.SyncRosterAliasesFromPlayerStats();

    PlayerSS->MarkDirty();
    PlayerSS->SavePlayer(true);

    selected_item = nullptr;

    RefreshRoster();
    ShowPlayer();
}

void UPlayerDlg::OnApply()
{
    UStarshatterPlayerSubsystem* PlayerSS = GetPlayerSubsystem();

    UpdatePlayer();

    if (PlayerSS)
        PlayerSS->SavePlayer(false);

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
    if (UnixSeconds <= 0)
        return TEXT("");

    const FDateTime DT = FDateTime::FromUnixTimestamp(UnixSeconds);
    return DT.ToString(TEXT("%Y-%m-%d"));
}
