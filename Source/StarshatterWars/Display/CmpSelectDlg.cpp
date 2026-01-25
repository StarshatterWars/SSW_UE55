/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright © 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         CmpSelectDlg.cpp
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    UNREAL PORT:
    - Converted from FormWindow/AWEvent mapping to UBaseScreen (UUserWidget-derived).
    - Preserves original member names and intent where applicable.
    - Removes MemDebug and allocation tags.
    - Converts Print-style debugging to UE_LOG.
*/

#include "CmpSelectDlg.h"

// UMG:
#include "Components/Button.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"

// Starshatter (ported core/gameplay):
#include "MenuScreen.h"
#include "Starshatter.h"
#include "Campaign.h"
#include "CampaignSaveGame.h"
#include "CombatGroup.h"
#include "PlayerCharacter.h"

#include "Game.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "FormatUtil.h"

// Optional: only if you use Bitmap thumbnails in this file:
#include "Bitmap.h"

UCmpSelectDlg::UCmpSelectDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UCmpSelectDlg::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // Cache core singletons/text:
    stars = Starshatter::GetInstance();
    select_msg = Game::GetText("CmpSelectDlg.select_msg");

    RegisterControls();
}

void UCmpSelectDlg::NativeConstruct()
{
    Super::NativeConstruct();
}

void UCmpSelectDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    ExecFrame();
}

// +--------------------------------------------------------------------+

void UCmpSelectDlg::RegisterControls()
{
    // Buttons:
    if (btn_new)    btn_new->OnClicked.AddDynamic(this, &UCmpSelectDlg::OnNew);
    if (btn_saved)  btn_saved->OnClicked.AddDynamic(this, &UCmpSelectDlg::OnSaved);

    if (btn_delete)
    {
        btn_delete->SetIsEnabled(false);
        btn_delete->OnClicked.AddDynamic(this, &UCmpSelectDlg::OnDelete);
    }

    if (btn_accept)
    {
        btn_accept->SetIsEnabled(false);
        btn_accept->OnClicked.AddDynamic(this, &UCmpSelectDlg::OnAccept);
    }

    if (btn_cancel)
    {
        btn_cancel->SetIsEnabled(true);
        btn_cancel->OnClicked.AddDynamic(this, &UCmpSelectDlg::OnCancel);
    }

    // List selection:
    if (lst_campaigns)
    {
        // NOTE: UCmpSelectDlg::OnCampaignSelect MUST have signature:
        // UFUNCTION() void OnCampaignSelect(UObject* SelectedItem);
        lst_campaigns->OnItemSelectionChanged().AddUObject(this, &UCmpSelectDlg::OnCampaignSelect);
    }

    // Default view:
    ShowNewCampaigns();
}

// +--------------------------------------------------------------------+

void UCmpSelectDlg::ExecFrame()
{
    if (Keyboard::KeyDown(VK_RETURN))
    {
        if (btn_accept && btn_accept->GetIsEnabled())
            OnAccept();
    }

    AutoThreadSync a(sync);

    if (!loaded)
        return;

    loaded = false;

    if (btn_cancel)
        btn_cancel->SetIsEnabled(true);

    if (!description || !btn_accept)
        return;

    if (campaign)
    {
        Campaign::SelectCampaign(campaign->Name());

        // For "new campaign" list index, update preview image to selected-state:
        if (load_index >= 0)
        {
            if (load_index >= 0 && load_index < images.size())
                images[load_index]->CopyBitmap(*campaign->GetImage(1));
        }

        // Build description text:
        if (load_index >= 0)
        {
            const Text desc =
                Text("<font Limerick12><color ffffff>") +
                campaign->Name() +
                Text("<font Verdana>\n\n") +
                Text("<color ffff80>") +
                Game::GetText("CmpSelectDlg.scenario") +
                Text("<color ffffff>\n\t") +
                campaign->Description();

            description->SetText(FText::FromString(UTF8_TO_TCHAR(desc.data())));
        }
        else
        {
            char time_buf[32];
            char score_buf[32];

            const double t = campaign->GetLoadTime() - campaign->GetStartTime();
            FormatDayTime(time_buf, t);
            sprintf_s(score_buf, "%d", campaign->GetPlayerTeamScore());

            Text desc =
                Text("<font Limerick12><color ffffff>") +
                campaign->Name() +
                Text("<font Verdana>\n\n") +
                Text("<color ffff80>") +
                Game::GetText("CmpSelectDlg.scenario") +
                Text("<color ffffff>\n\t") +
                campaign->Description() +
                Text("\n\n<color ffff80>") +
                Game::GetText("CmpSelectDlg.campaign-time") +
                Text("<color ffffff>\n\t") +
                time_buf +
                Text("\n\n<color ffff80>") +
                Game::GetText("CmpSelectDlg.assignment") +
                Text("<color ffffff>\n\t");

            if (campaign->GetPlayerGroup())
                desc += campaign->GetPlayerGroup()->GetDescription();
            else
                desc += "n/a";

            desc +=
                Text("\n\n<color ffff80>") +
                Game::GetText("CmpSelectDlg.team-score") +
                Text("<color ffffff>\n\t") +
                score_buf;

            description->SetText(FText::FromString(UTF8_TO_TCHAR(desc.data())));
        }

        btn_accept->SetIsEnabled(true);

        if (btn_delete)
            btn_delete->SetIsEnabled(show_saved);
    }
    else
    {
        description->SetText(FText::FromString(UTF8_TO_TCHAR(select_msg.data())));
        btn_accept->SetIsEnabled(true);

        if (btn_delete)
            btn_delete->SetIsEnabled(false);
    }
}

bool UCmpSelectDlg::CanClose()
{
    AutoThreadSync a(sync);
    return !loading;
}

// +--------------------------------------------------------------------+

void UCmpSelectDlg::Show()
{
    ShowNewCampaigns();
}

// +--------------------------------------------------------------------+

void UCmpSelectDlg::ShowNewCampaigns()
{
    AutoThreadSync a(sync);

    if (loading && description)
    {
        description->SetText(FText::FromString(UTF8_TO_TCHAR(
            Game::GetText("CmpSelectDlg.already-loading").data())));
        return;
    }

    if (btn_delete)
        btn_delete->SetIsEnabled(false);

    if (lst_campaigns)
    {
        // NOTE: If you haven’t implemented UObject list items yet, do NOT call AddItem with strings.
        // You must create UObject items. Keep your current placeholder approach.

        images.destroy();

        PlayerCharacter* player = PlayerCharacter::GetCurrentPlayer();
        if (!player)
            return;

        ListIter<Campaign> iter = Campaign::GetAllCampaigns();
        while (++iter)
        {
            Campaign* c = iter.value();
            if (!c)
                continue;

            if (c->GetCampaignId() < Campaign::SINGLE_MISSIONS)
            {
                Bitmap* bmp = new Bitmap;
                bmp->CopyBitmap(*c->GetImage(0));
                images.append(bmp);

                const int cid = c->GetCampaignId();
                const bool locked_full =
                    (cid > 2 && cid < 10 && !player->HasCompletedCampaign(cid - 1));

                const bool locked_extra =
                    (cid >= 10 && cid < 30 && (cid % 10) != 0 && !player->HasCompletedCampaign(cid - 1));

                if (locked_full || locked_extra)
                {
                    const int n = images.size() - 1;
                    images[n]->CopyBitmap(*c->GetImage(2));
                }

                // TODO: create and add UObject list items here (UCmpSelectItem etc.)
            }
        }
    }

    if (description)
        description->SetText(FText::FromString(UTF8_TO_TCHAR(select_msg.data())));

    if (btn_accept)
        btn_accept->SetIsEnabled(false);

    show_saved = false;
}

// +--------------------------------------------------------------------+

void UCmpSelectDlg::ShowSavedCampaigns()
{
    AutoThreadSync a(sync);

    if (loading && description)
    {
        description->SetText(FText::FromString(UTF8_TO_TCHAR(
            Game::GetText("CmpSelectDlg.already-loading").data())));
        return;
    }

    if (btn_delete)
        btn_delete->SetIsEnabled(false);

    if (lst_campaigns)
    {
        List<Text> save_list;
        CampaignSaveGame::GetSaveGameList(save_list);
        save_list.sort();

        // TODO: create and add UObject list items here (saved campaigns).
        save_list.destroy();
    }

    if (description)
        description->SetText(FText::FromString(UTF8_TO_TCHAR(select_msg.data())));

    if (btn_accept)
        btn_accept->SetIsEnabled(false);

    show_saved = true;
}

// +--------------------------------------------------------------------+

void UCmpSelectDlg::OnCampaignSelect(UObject* SelectedItem)
{
    if (description && lst_campaigns)
    {
        AutoThreadSync a(sync);

        if (loading)
        {
            description->SetText(FText::FromString(UTF8_TO_TCHAR(
                Game::GetText("CmpSelectDlg.already-loading").data())));
            return;
        }

        load_index = -1;
        load_file = "";

        // TODO: map SelectedItem -> (load_index or load_file)
        // Example:
        // UCmpSelectItem* Item = Cast<UCmpSelectItem>(SelectedItem);
        // if (Item) { load_index = Item->LoadIndex; load_file = TCHAR_TO_UTF8(*Item->SaveFile); }

        if (btn_accept)
            btn_accept->SetIsEnabled(false);
    }

    if (!loading && (load_index >= 0 || load_file.length() > 0))
    {
        if (btn_cancel)
            btn_cancel->SetIsEnabled(false);

        StartLoadProc();
    }
}

// +--------------------------------------------------------------------+

void UCmpSelectDlg::OnNew()
{
    ShowNewCampaigns();
}

void UCmpSelectDlg::OnSaved()
{
    ShowSavedCampaigns();
}

void UCmpSelectDlg::OnDelete()
{
    load_file = "";

    if (load_file.length())
    {
        UE_LOG(LogTemp, Verbose, TEXT("CmpSelectDlg: Request delete for save '%s'"),
            UTF8_TO_TCHAR(load_file.data()));
    }

    ShowSavedCampaigns();
}

void UCmpSelectDlg::OnConfirmDelete()
{
    if (load_file.length())
        CampaignSaveGame::Delete(load_file);

    ShowSavedCampaigns();
}

// +--------------------------------------------------------------------+

void UCmpSelectDlg::OnAccept()
{
    AutoThreadSync a(sync);

    if (loading)
        return;

    if (!show_saved)
        Campaign::GetCampaign()->Load();
    else
        Game::ResetGameTime();

    Mouse::Show(false);

    if (stars)
        stars->SetGameMode(Starshatter::CLOD_MODE);
}

void UCmpSelectDlg::OnCancel()
{
    UE_LOG(LogTemp, Verbose, TEXT("CmpSelectDlg: Cancel -> show menu"));
}

// +--------------------------------------------------------------------+
// Loading (synchronous legacy-style port)
// +--------------------------------------------------------------------+

void UCmpSelectDlg::StartLoadProc()
{
    if (hproc != nullptr)
        return;

    campaign = nullptr;
    loading = true;
    loaded = false;

    if (description)
        description->SetText(FText::FromString(UTF8_TO_TCHAR(
            Game::GetText("CmpSelectDlg.loading").data())));

    const uint32 Result = LoadProc();
    UE_LOG(LogTemp, Verbose, TEXT("CmpSelectDlg: LoadProc result=%u"), Result);
}

void UCmpSelectDlg::StopLoadProc()
{
    hproc = nullptr;
}

uint32 UCmpSelectDlg::LoadProc()
{
    Campaign* c = nullptr;

    if (load_index >= 0)
    {
        List<Campaign>& list = Campaign::GetAllCampaigns();
        if (load_index < list.size())
        {
            c = list[load_index];
            if (c) c->Load();
        }
    }
    else
    {
        CampaignSaveGame savegame;
        savegame.Load(load_file);
        c = savegame.GetCampaign();
    }

    sync.acquire();
    loading = false;
    loaded = true;
    campaign = c;
    sync.release();

    return 0;
}
