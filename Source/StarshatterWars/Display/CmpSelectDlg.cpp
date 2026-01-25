/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         CmpSelectDlg.cpp
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    Campaign Select dialog — Unreal-compatible port of the legacy CmpSelectDlg.
    This is a *logic-preserving* port. It retains the legacy UI control model
    (FormWindow/Button/ListBox/etc.) and threading pattern (HANDLE + CreateThread).
    Replace CreateThread/WaitForSingleObject with FRunnable/Async if desired later.

    NOTES
    =====
    - Keeps all variables and method names/signatures as in the legacy file.
    - Removes MemDebug new(__FILE__,__LINE__) instrumentation.
    - Preserves Bitmap/List usage as the legacy UI layer appears to still exist.
*/

#include "CmpSelectDlg.h"

#include "ConfirmDlg.h"
#include "MenuScreen.h"
#include "Starshatter.h"
#include "Campaign.h"
#include "CampaignSaveGame.h"
#include "CombatGroup.h"
#include "ShipDesign.h"
#include "Player.h"

#include "Game.h"
#include "DataLoader.h"
#include "Button.h"
#include "ListBox.h"
#include "Slider.h"
#include "Video.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "ParseUtil.h"
#include "FormatUtil.h"

// Unreal:
#include "CoreMinimal.h"

DEFINE_LOG_CATEGORY_STATIC(LogCmpSelectDlg, Log, All);

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:

DEF_MAP_CLIENT(CmpSelectDlg, OnNew);
DEF_MAP_CLIENT(CmpSelectDlg, OnSaved);
DEF_MAP_CLIENT(CmpSelectDlg, OnDelete);
DEF_MAP_CLIENT(CmpSelectDlg, OnConfirmDelete);
DEF_MAP_CLIENT(CmpSelectDlg, OnAccept);
DEF_MAP_CLIENT(CmpSelectDlg, OnCancel);
DEF_MAP_CLIENT(CmpSelectDlg, OnCampaignSelect);

// +--------------------------------------------------------------------+

CmpSelectDlg::CmpSelectDlg(Screen* s, FormDef& def, MenuScreen* mgr)
    : FormWindow(s, 0, 0, s->Width(), s->Height())
    , manager(mgr)
    , btn_new(nullptr)
    , btn_saved(nullptr)
    , btn_delete(nullptr)
    , btn_accept(nullptr)
    , btn_cancel(nullptr)
    , lst_campaigns(nullptr)
    , description(nullptr)
    , stars(nullptr)
    , campaign(nullptr)
    , selected_mission(-1)
    , hproc(0)
    , loading(false)
    , loaded(false)
    , load_index(-1)
    , show_saved(false)
{
    stars = Starshatter::GetInstance();
    select_msg = Game::GetText("CmpSelectDlg.select_msg");
    Init(def);
}

CmpSelectDlg::~CmpSelectDlg()
{
    StopLoadProc();
    images.destroy();
}

// +--------------------------------------------------------------------+

void
CmpSelectDlg::RegisterControls()
{
    btn_new = (Button*)FindControl(100);
    btn_saved = (Button*)FindControl(101);
    btn_delete = (Button*)FindControl(102);
    btn_accept = (Button*)FindControl(1);
    btn_cancel = (Button*)FindControl(2);

    if (btn_new)
        REGISTER_CLIENT(EID_CLICK, btn_new, CmpSelectDlg, OnNew);

    if (btn_saved)
        REGISTER_CLIENT(EID_CLICK, btn_saved, CmpSelectDlg, OnSaved);

    if (btn_delete) {
        btn_delete->SetEnabled(false);
        REGISTER_CLIENT(EID_CLICK, btn_delete, CmpSelectDlg, OnDelete);
        REGISTER_CLIENT(EID_USER_1, btn_delete, CmpSelectDlg, OnConfirmDelete);
    }

    if (btn_accept) {
        btn_accept->SetEnabled(false);
        REGISTER_CLIENT(EID_CLICK, btn_accept, CmpSelectDlg, OnAccept);
    }

    if (btn_cancel) {
        btn_cancel->SetEnabled(true);
        REGISTER_CLIENT(EID_CLICK, btn_cancel, CmpSelectDlg, OnCancel);
    }

    description = FindControl(200);
    lst_campaigns = (ListBox*)FindControl(201);

    if (lst_campaigns)
        REGISTER_CLIENT(EID_SELECT, lst_campaigns, CmpSelectDlg, OnCampaignSelect);

    ShowNewCampaigns();
}

// +--------------------------------------------------------------------+

void
CmpSelectDlg::ExecFrame()
{
    if (Keyboard::KeyDown(VK_RETURN)) {
        if (btn_accept && btn_accept->IsEnabled())
            OnAccept(nullptr);
    }

    AutoThreadSync a(sync);

    if (loaded) {
        loaded = false;

        if (btn_cancel)
            btn_cancel->SetEnabled(true);

        if (description && btn_accept) {
            if (campaign) {
                Campaign::SelectCampaign(campaign->Name());

                if (load_index >= 0) {
                    if (lst_campaigns) {
                        images[load_index]->CopyBitmap(*campaign->GetImage(1));
                        lst_campaigns->SetItemImage(load_index, images[load_index]);
                    }

                    description->SetText(
                        Text("<font Limerick12><color ffffff>") +
                        campaign->Name() +
                        Text("<font Verdana>\n\n") +
                        Text("<color ffff80>") +
                        Game::GetText("CmpSelectDlg.scenario") +
                        Text("<color ffffff>\n\t") +
                        campaign->Description()
                    );
                }
                else {
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

                    description->SetText(desc);
                }

                btn_accept->SetEnabled(true);

                if (btn_delete)
                    btn_delete->SetEnabled(show_saved);
            }
            else {
                description->SetText(select_msg);
                btn_accept->SetEnabled(true);
            }
        }
    }
}

bool
CmpSelectDlg::CanClose()
{
    AutoThreadSync a(sync);
    return !loading;
}

// +--------------------------------------------------------------------+

void
CmpSelectDlg::ShowNewCampaigns()
{
    AutoThreadSync a(sync);

    if (loading && description) {
        description->SetText(Game::GetText("CmpSelectDlg.already-loading"));
        Button::PlaySound(Button::SND_REJECT);
        return;
    }

    if (btn_new)   btn_new->SetButtonState(1);
    if (btn_saved) btn_saved->SetButtonState(0);
    if (btn_delete) btn_delete->SetEnabled(false);

    if (lst_campaigns) {
        images.destroy();

        lst_campaigns->SetSelectedStyle(ListBox::LIST_ITEM_STYLE_PLAIN);
        lst_campaigns->SetLeading(0);
        lst_campaigns->ClearItems();
        lst_campaigns->SetLineHeight(100);

        Player* player = Player::GetCurrentPlayer();
        if (!player) return;

        ListIter<Campaign> iter = Campaign::GetAllCampaigns();
        while (++iter) {
            Campaign* c = iter.value();
            if (!c) continue;

            if (c->GetCampaignId() < Campaign::SINGLE_MISSIONS) {
                Bitmap* bmp = new Bitmap;
                bmp->CopyBitmap(*c->GetImage(0));
                images.append(bmp);

                const int n = lst_campaigns->AddImage(bmp) - 1;
                lst_campaigns->SetItemText(n, c->Name());

                // FULL GAME CRITERIA (based on player record):
                if (c->GetCampaignId() > 2 && c->GetCampaignId() < 10 &&
                    !player->HasCompletedCampaign(c->GetCampaignId() - 1)) {
                    images[n]->CopyBitmap(*c->GetImage(2));
                    lst_campaigns->SetItemImage(n, images[n]);
                }

                // Two additional sequences of ten campaigns (10-19 and 20-29)
                else if (c->GetCampaignId() >= 10 && c->GetCampaignId() < 30 &&
                    (c->GetCampaignId() % 10) != 0 &&
                    !player->HasCompletedCampaign(c->GetCampaignId() - 1)) {
                    images[n]->CopyBitmap(*c->GetImage(2));
                    lst_campaigns->SetItemImage(n, images[n]);
                }

                // NOTE: Campaigns 10, 20, and 30-99 are always enabled if they exist!
            }
        }
    }

    if (description)
        description->SetText(select_msg);

    if (btn_accept)
        btn_accept->SetEnabled(false);

    show_saved = false;
}

// +--------------------------------------------------------------------+

void
CmpSelectDlg::ShowSavedCampaigns()
{
    AutoThreadSync a(sync);

    if (loading && description) {
        description->SetText(Game::GetText("CmpSelectDlg.already-loading"));
        Button::PlaySound(Button::SND_REJECT);
        return;
    }

    if (btn_new)   btn_new->SetButtonState(0);
    if (btn_saved) btn_saved->SetButtonState(1);
    if (btn_delete) btn_delete->SetEnabled(false);

    if (lst_campaigns) {
        lst_campaigns->SetSelectedStyle(ListBox::LIST_ITEM_STYLE_FILLED_BOX);
        lst_campaigns->SetLeading(4);
        lst_campaigns->ClearItems();
        lst_campaigns->SetLineHeight(12);

        List<Text> save_list;
        CampaignSaveGame::GetSaveGameList(save_list);
        save_list.sort();

        for (int i = 0; i < save_list.size(); ++i)
            lst_campaigns->AddItem(*save_list[i]);

        save_list.destroy();
    }

    if (description)
        description->SetText(select_msg);

    if (btn_accept)
        btn_accept->SetEnabled(false);

    show_saved = true;
}

// +--------------------------------------------------------------------+

void
CmpSelectDlg::OnCampaignSelect(AWEvent* event)
{
    (void)event;

    if (description && lst_campaigns) {
        AutoThreadSync a(sync);

        if (loading) {
            description->SetText(Game::GetText("CmpSelectDlg.already-loading"));
            Button::PlaySound(Button::SND_REJECT);
            return;
        }

        load_index = -1;
        load_file = "";

        Player* player = Player::GetCurrentPlayer();
        if (!player) return;

        // NEW CAMPAIGN:
        if (btn_new && btn_new->GetButtonState()) {
            List<Campaign>& list = Campaign::GetAllCampaigns();

            for (int i = 0; i < lst_campaigns->NumItems(); ++i) {
                Campaign* c = list[i];
                if (!c) continue;

                const bool available =
                    (c->GetCampaignId() <= 2) ||
                    player->HasCompletedCampaign(c->GetCampaignId() - 1);

                if (available) {
                    if (lst_campaigns->IsSelected(i)) {
                        images[i]->CopyBitmap(*c->GetImage(1));
                        lst_campaigns->SetItemImage(i, images[i]);

                        load_index = i;
                    }
                    else {
                        images[i]->CopyBitmap(*c->GetImage(0));
                        lst_campaigns->SetItemImage(i, images[i]);
                    }
                }
                else {
                    images[i]->CopyBitmap(*c->GetImage(2));
                    lst_campaigns->SetItemImage(i, images[i]);

                    if (lst_campaigns->IsSelected(i)) {
                        description->SetText(select_msg);
                    }
                }
            }
        }

        // SAVED CAMPAIGN:
        else {
            const int seln = lst_campaigns->GetSelection();

            if (seln < 0) {
                description->SetText(select_msg);
            }
            else {
                load_index = -1;
                load_file = lst_campaigns->GetItemText(seln);
            }
        }

        if (btn_accept)
            btn_accept->SetEnabled(false);
    }

    if (!loading && (load_index >= 0 || load_file.length() > 0)) {
        if (btn_cancel)
            btn_cancel->SetEnabled(false);

        StartLoadProc();
    }
}

// +--------------------------------------------------------------------+

void
CmpSelectDlg::Show()
{
    FormWindow::Show();
    ShowNewCampaigns();
}

// +--------------------------------------------------------------------+

void
CmpSelectDlg::OnNew(AWEvent* event)
{
    (void)event;
    ShowNewCampaigns();
}

void
CmpSelectDlg::OnSaved(AWEvent* event)
{
    (void)event;
    ShowSavedCampaigns();
}

void
CmpSelectDlg::OnDelete(AWEvent* event)
{
    (void)event;

    load_file = "";

    if (lst_campaigns) {
        const int seln = lst_campaigns->GetSelection();

        if (seln < 0) {
            if (description) description->SetText(select_msg);
            if (btn_accept)  btn_accept->SetEnabled(false);
        }
        else {
            load_index = -1;
            load_file = lst_campaigns->GetItemText(seln);
        }
    }

    if (load_file.length()) {
        ConfirmDlg* confirm = manager ? manager->GetConfirmDlg() : nullptr;

        if (confirm) {
            char msg[256];
            sprintf_s(msg, Game::GetText("CmpSelectDlg.are-you-sure"), load_file.data());
            confirm->SetMessage(msg);
            confirm->SetTitle(Game::GetText("CmpSelectDlg.confirm"));

            manager->ShowConfirmDlg();
        }
        else {
            OnConfirmDelete(event);
        }
    }

    ShowSavedCampaigns();
}

void
CmpSelectDlg::OnConfirmDelete(AWEvent* event)
{
    (void)event;

    if (load_file.length()) {
        CampaignSaveGame::Delete(load_file);
    }

    ShowSavedCampaigns();
}

// +--------------------------------------------------------------------+

void
CmpSelectDlg::OnAccept(AWEvent* event)
{
    (void)event;

    AutoThreadSync a(sync);

    if (loading)
        return;

    // If this is to be a new campaign, re-instantiate the campaign object:
    if (btn_new && btn_new->GetButtonState())
        Campaign::GetCampaign()->Load();
    else
        Game::ResetGameTime();

    Mouse::Show(false);

    if (stars)
        stars->SetGameMode(Starshatter::CLOD_MODE);
}

void
CmpSelectDlg::OnCancel(AWEvent* event)
{
    (void)event;

    if (manager)
        manager->ShowMenuDlg();
}

// +--------------------------------------------------------------------+

DWORD WINAPI CmpSelectDlgLoadProc(LPVOID link);

void
CmpSelectDlg::StartLoadProc()
{
    if (hproc != 0) {
        DWORD result = 0;
        GetExitCodeThread(hproc, &result);

        if (result != STILL_ACTIVE) {
            CloseHandle(hproc);
            hproc = 0;
        }
        else {
            return;
        }
    }

    if (hproc == 0) {
        campaign = 0;
        loading = true;
        loaded = false;

        if (description)
            description->SetText(Game::GetText("CmpSelectDlg.loading"));

        DWORD thread_id = 0;
        hproc = CreateThread(0, 4096, CmpSelectDlgLoadProc, (LPVOID)this, 0, &thread_id);

        if (hproc == 0) {
            static int report = 10;
            if (report > 0) {
                UE_LOG(LogCmpSelectDlg, Warning, TEXT("CmpSelectDlg failed to create thread (err=%08x)"), (uint32)GetLastError());
                report--;

                if (report == 0) {
                    UE_LOG(LogCmpSelectDlg, Warning, TEXT("Further warnings of this type will be suppressed."));
                }
            }
        }
    }
}

void
CmpSelectDlg::StopLoadProc()
{
    if (hproc != 0) {
        WaitForSingleObject(hproc, 2500);
        CloseHandle(hproc);
        hproc = 0;
    }
}

DWORD WINAPI
CmpSelectDlgLoadProc(LPVOID link)
{
    CmpSelectDlg* dlg = (CmpSelectDlg*)link;

    if (dlg)
        return dlg->LoadProc();

    return (DWORD)E_POINTER;
}

DWORD
CmpSelectDlg::LoadProc()
{
    Campaign* c = 0;

    // NEW CAMPAIGN:
    if (load_index >= 0) {
        List<Campaign>& list = Campaign::GetAllCampaigns();

        if (load_index < list.size()) {
            c = list[load_index];
            if (c) c->Load();
        }
    }

    // SAVED CAMPAIGN:
    else {
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

// +--------------------------------------------------------------------+
