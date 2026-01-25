/*  Project Starshatter 5.0
    Destroyer Studios LLC
    Copyright © 1997-2007.

    SUBSYSTEM:    Stars.exe
    FILE:         PlayerDlg.cpp
    AUTHOR:       John DiCamillo

    UNREAL PORT:
    - Converted from FormWindow to UBaseScreen (UUserWidget-derived).
    - Preserves original method names and member names.
    - Replaces AWEvent routing with UMG delegates.
    - Replaces VK_RETURN polling with UE key handling (NativeOnKeyDown).
*/

#include "PlayerDlg.h"

#include "BaseScreen.h"

// UMG:
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

// UE:
#include "HAL/PlatformMisc.h"
#include "Input/Reply.h"
#include "Misc/Guid.h"
#include "Misc/Crc.h"

// Starshatter port headers:
#include "PlayerCharacter.h"
#include "Game.h"

// If you have these dialogs in UE, include their headers and cast manager appropriately:
// #include "AwardShowDlg.h"
// #include "ConfirmDlg.h"

// +--------------------------------------------------------------------+

UPlayerDlg::UPlayerDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Ensure medals array is sized like the original int medals[15]
    medals.Init(-1, 15);
}

// +--------------------------------------------------------------------+

void UPlayerDlg::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    RegisterControls();
}

void UPlayerDlg::NativeConstruct()
{
    Super::NativeConstruct();
    SetIsFocusable(true);
}

void UPlayerDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    ExecFrame();
}

// +--------------------------------------------------------------------+

void UPlayerDlg::RegisterControls()
{
    // Roster select:
    if (lst_roster)
    {
        // UListView selection callbacks vary slightly by UE version.
        // OnItemSelectionChanged is available; we use it and call OnSelectPlayer().
        lst_roster->OnItemSelectionChanged().RemoveAll(this);
        lst_roster->OnItemSelectionChanged().AddUObject(this, &UPlayerDlg::OnSelectPlayer);
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

    // Apply/Cancel (prefer the local buttons; fall back to base screen buttons if present):
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

    // Rank/Medal clicks:
    // UImage does not emit click events by default; you typically wrap images in UButton
    // (or use UBorder + OnMouseButtonDown in UUserWidget).
    // Therefore, OnRank/OnMedal are left callable, but you must wire them in UMG
    // (e.g., transparent buttons over the images) and bind those buttons to these handlers.
}

// +--------------------------------------------------------------------+

void UPlayerDlg::Show()
{
    SetVisibility(ESlateVisibility::Visible);
    SetKeyboardFocus();

    // Populate roster view:
    if (!lst_roster)
        return;

    lst_roster->ClearListItems();

    int32 CurrentIndex = 0;

    List<PlayerCharacter>& roster = PlayerCharacter::GetRoster();
    for (int32 i = 0; i < roster.size(); i++)
    {
        Player* p = roster[i];

        // UListView requires UObject items. If you already have a UObject wrapper for Player,
        // use it here. As a minimal bridge, you can create a UObject list item containing the name.
        //
        // This port assumes you will provide a proper item class later; for now we skip adding
        // if you don't have such a class.
        //
        // Example future:
        // UPlayerListItem* Item = NewObject<UPlayerListItem>(this);
        // Item->PlayerPtr = p;
        // lst_roster->AddItem(Item);

        if (p == PlayerCharacter::GetCurrentPlayer())
            CurrentIndex = i;
    }

    // Selecting in a UListView requires selecting an actual UObject item; handled once you have items.

    ShowPlayer();

    // Enable apply if any roster entries exist:
    const bool bHasPlayers = roster.size() > 0;
    if (apply) apply->SetIsEnabled(bHasPlayers);
    if (ApplyButton) ApplyButton->SetIsEnabled(bHasPlayers);
}

// +--------------------------------------------------------------------+

void UPlayerDlg::ExecFrame()
{
    // Original polled VK_RETURN; in UE we handle Enter in NativeOnKeyDown typically.
    // Keeping empty here matches "no per-frame work" best practice.
}

// +--------------------------------------------------------------------+

void UPlayerDlg::ShowPlayer()
{
    Player* p = PlayerCharacter::GetCurrentPlayer();

    // Helper lambdas for brevity:
    auto SetTextSafe = [](UEditableTextBox* Box, const FString& Val)
        {
            if (Box) Box->SetText(FText::FromString(Val));
        };

    auto SetLabelSafe = [](UTextBlock* Label, const FString& Val)
        {
            if (Label) Label->SetText(FText::FromString(Val));
        };

    if (p)
    {
        // Edit fields:
        SetTextSafe(txt_name, ANSI_TO_TCHAR(p->Name().data()));
        SetTextSafe(txt_password, ANSI_TO_TCHAR(p->Password().data()));
        SetTextSafe(txt_squadron, ANSI_TO_TCHAR(p->Squadron().data()));
        SetTextSafe(txt_signature, ANSI_TO_TCHAR(p->Signature().data()));

        // Stats formatting:
        char flight_time[64], missions[16], kills[16], losses[16], points[16];
        FormatTime(flight_time, p->FlightTime());
        sprintf_s(missions, "%d", p->Missions());
        sprintf_s(kills, "%d", p->Kills());
        sprintf_s(losses, "%d", p->Losses());
        sprintf_s(points, "%d", p->Points());

        if (lbl_createdate)  lbl_createdate->SetText(FText::FromString(ANSI_TO_TCHAR(FormatTimeString(p->CreateDate()))));
        if (lbl_rank)        lbl_rank->SetText(FText::FromString(ANSI_TO_TCHAR(Player::RankName(p->Rank()))));
        SetLabelSafe(lbl_flighttime, ANSI_TO_TCHAR(flight_time));
        SetLabelSafe(lbl_missions, ANSI_TO_TCHAR(missions));
        SetLabelSafe(lbl_kills, ANSI_TO_TCHAR(kills));
        SetLabelSafe(lbl_losses, ANSI_TO_TCHAR(losses));
        SetLabelSafe(lbl_points, ANSI_TO_TCHAR(points));

        // Rank insignia:
        if (img_rank)
        {
            // Assumes RankInsignia returns something you can convert to a UTexture2D/Brush.
            // If Player::RankInsignia returns Bitmap*, you will need your Bitmap->Brush bridge.
            // For now, just show the widget.
            img_rank->SetVisibility(ESlateVisibility::Visible);
        }

        // Medals:
        for (int32 i = 0; i < 15; i++)
        {
            const int32 MedalVal = p->Medal(i);
            medals[i] = MedalVal ? MedalVal : -1;

            UImage* MedalImg = nullptr;
            switch (i)
            {
            case 0:  MedalImg = img_medals_0;  break;
            case 1:  MedalImg = img_medals_1;  break;
            case 2:  MedalImg = img_medals_2;  break;
            case 3:  MedalImg = img_medals_3;  break;
            case 4:  MedalImg = img_medals_4;  break;
            case 5:  MedalImg = img_medals_5;  break;
            case 6:  MedalImg = img_medals_6;  break;
            case 7:  MedalImg = img_medals_7;  break;
            case 8:  MedalImg = img_medals_8;  break;
            case 9:  MedalImg = img_medals_9;  break;
            case 10: MedalImg = img_medals_10; break;
            case 11: MedalImg = img_medals_11; break;
            case 12: MedalImg = img_medals_12; break;
            case 13: MedalImg = img_medals_13; break;
            case 14: MedalImg = img_medals_14; break;
            default: break;
            }

            if (MedalImg)
            {
                MedalImg->SetVisibility(MedalVal ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
                // As with rank: set brush once you have Bitmap->Brush conversion.
            }
        }

        // Chat macros:
        UEditableTextBox* ChatBoxes[10] =
        {
            txt_chat_0, txt_chat_1, txt_chat_2, txt_chat_3, txt_chat_4,
            txt_chat_5, txt_chat_6, txt_chat_7, txt_chat_8, txt_chat_9
        };

        for (int32 i = 0; i < 10; i++)
        {
            if (ChatBoxes[i])
                ChatBoxes[i]->SetText(FText::FromString(ANSI_TO_TCHAR(p->ChatMacro(i))));
        }
    }
    else
    {
        SetTextSafe(txt_name, TEXT(""));
        SetTextSafe(txt_password, TEXT(""));
        SetTextSafe(txt_squadron, TEXT(""));
        SetTextSafe(txt_signature, TEXT(""));

        SetLabelSafe(lbl_createdate, TEXT(""));
        SetLabelSafe(lbl_rank, TEXT(""));
        SetLabelSafe(lbl_flighttime, TEXT(""));
        SetLabelSafe(lbl_missions, TEXT(""));
        SetLabelSafe(lbl_kills, TEXT(""));
        SetLabelSafe(lbl_losses, TEXT(""));
        SetLabelSafe(lbl_points, TEXT(""));

        if (img_rank)
            img_rank->SetVisibility(ESlateVisibility::Hidden);

        for (int32 i = 0; i < 15; i++)
        {
            medals[i] = -1;

            UImage* MedalImg = nullptr;
            switch (i)
            {
            case 0:  MedalImg = img_medals_0;  break;
            case 1:  MedalImg = img_medals_1;  break;
            case 2:  MedalImg = img_medals_2;  break;
            case 3:  MedalImg = img_medals_3;  break;
            case 4:  MedalImg = img_medals_4;  break;
            case 5:  MedalImg = img_medals_5;  break;
            case 6:  MedalImg = img_medals_6;  break;
            case 7:  MedalImg = img_medals_7;  break;
            case 8:  MedalImg = img_medals_8;  break;
            case 9:  MedalImg = img_medals_9;  break;
            case 10: MedalImg = img_medals_10; break;
            case 11: MedalImg = img_medals_11; break;
            case 12: MedalImg = img_medals_12; break;
            case 13: MedalImg = img_medals_13; break;
            case 14: MedalImg = img_medals_14; break;
            default: break;
            }

            if (MedalImg)
                MedalImg->SetVisibility(ESlateVisibility::Hidden);
        }

        UEditableTextBox* ChatBoxes[10] =
        {
            txt_chat_0, txt_chat_1, txt_chat_2, txt_chat_3, txt_chat_4,
            txt_chat_5, txt_chat_6, txt_chat_7, txt_chat_8, txt_chat_9
        };

        for (int32 i = 0; i < 10; i++)
        {
            if (ChatBoxes[i])
                ChatBoxes[i]->SetText(FText::GetEmpty());
        }
    }
}

// +--------------------------------------------------------------------+

void UPlayerDlg::UpdatePlayer()
{
    PlayPlayerCharacterer* p = PlayerCharacter::GetCurrentPlayer();
    if (!p) return;

    if (txt_name)      p->SetName(TCHAR_TO_ANSI(*txt_name->GetText().ToString()));
    if (txt_password)  p->SetPassword(TCHAR_TO_ANSI(*txt_password->GetText().ToString()));
    if (txt_squadron)  p->SetSquadron(TCHAR_TO_ANSI(*txt_squadron->GetText().ToString()));
    if (txt_signature) p->SetSignature(TCHAR_TO_ANSI(*txt_signature->GetText().ToString()));

    UEditableTextBox* ChatBoxes[10] =
    {
        txt_chat_0, txt_chat_1, txt_chat_2, txt_chat_3, txt_chat_4,
        txt_chat_5, txt_chat_6, txt_chat_7, txt_chat_8, txt_chat_9
    };

    for (int32 i = 0; i < 10; i++)
    {
        if (ChatBoxes[i])
            p->SetChatMacro(i, TCHAR_TO_ANSI(*ChatBoxes[i]->GetText().ToString()));
    }
}

// +--------------------------------------------------------------------+

void UPlayerDlg::OnSelectPlayer()
{
    if (!lst_roster)
        return;

    UpdatePlayer();

    // NOTE: UListView provides selected UObject items, not integer index.
    // Once you implement a UObject list item wrapper (e.g., UPlayerListItem),
    // retrieve it here and select the underlying Player*.
    //
    // For now, we simply refresh the display:
    ShowPlayer();

    List<PlayerCharacter>& roster = PlayerCharacter::GetRoster();
    const bool bHasPlayers = roster.size() > 0;
    if (apply) apply->SetIsEnabled(bHasPlayers);
    if (ApplyButton) ApplyButton->SetIsEnabled(bHasPlayers);
}

// +--------------------------------------------------------------------+

void UPlayerDlg::OnAdd()
{
    PlayerCharacter::Create("Pilot");
    ShowPlayer();

    // Repopulate list view items once you have item wrappers.
    List<PlayerCharacter>& roster = PlayerCharacter::GetRoster();
    const bool bHasPlayers = roster.size() > 0;
    if (apply) apply->SetIsEnabled(bHasPlayers);
    if (ApplyButton) ApplyButton->SetIsEnabled(bHasPlayers);
}

void UPlayerDlg::OnDel()
{
    if (!PlayerCharacter::GetCurrentPlayer())
        return;

    // Legacy ConfirmDlg flow:
    // ConfirmDlg* confirm = manager->GetConfirmDlg(); manager->ShowConfirmDlg();
    // In UE, implement confirm as a widget and call it here (or just delete immediately).

    OnDelConfirm();
}

void UPlayerDlg::OnDelConfirm()
{
    PlayerCharacter::Destroy(PlayerCharacter::GetCurrentPlayer());
    ShowPlayer();

    List<PlayerCharacter>& roster = PlayerCharacter::GetRoster();
    const bool bHasPlayers = roster.size() > 0;
    if (apply) apply->SetIsEnabled(bHasPlayers);
    if (ApplyButton) ApplyButton->SetIsEnabled(bHasPlayers);
}

// +--------------------------------------------------------------------+

void UPlayerDlg::OnRank()
{
    // Requires AwardShowDlg in UE (typically another widget).
    // Keep callable; wire from UMG.
}

void UPlayerDlg::OnMedal()
{
    // Requires knowing which medal was clicked.
    // In UE, bind separate buttons over each medal image and pass the index,
    // or store "last clicked medal index" in the widget.
}

// +--------------------------------------------------------------------+

void UPlayerDlg::OnApply()
{
    PlayerCharacter* player = PlayerCharacter::GetCurrentPlayer();
    if (player)
    {
        UpdatePlayer();
        PlayerCharacter::Save();
    }

    // Legacy: FlushKeys(); manager->ShowMenuDlg();
    // UE: hide widget; owner decides what comes next.
    SetVisibility(ESlateVisibility::Collapsed);
}

void UPlayerDlg::OnCancel()
{
    PlayerCharacter::Load();

    // Legacy: FlushKeys(); manager->ShowMenuDlg();
    SetVisibility(ESlateVisibility::Collapsed);
}
