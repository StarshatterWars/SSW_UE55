/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         CampaignSelectDlg.cpp
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

#include "CampaignSelectDlg.h"

#include "GameStructs.h"

// Unreal:
#include "CoreMinimal.h"
#include "Components/Button.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

// Starshatter (ported core/gameplay):
#include "ConfirmDlg.h"
#include "MenuScreen.h"
#include "Starshatter.h"
#include "Campaign.h"
#include "CampaignSaveGame.h"
#include "CombatGroup.h"
#include "ShipDesign.h"
#include "PlayerCharacter.h"

#include "Game.h"
#include "DataLoader.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "ParseUtil.h"
#include "FormatUtil.h"

// +--------------------------------------------------------------------+

UCampaignSelectDlg::UCampaignSelectDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // stars/select_msg initialized in NativeOnInitialized to ensure systems exist.
}

void UCampaignSelectDlg::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    stars = Starshatter::GetInstance();
    select_msg = Game::GetText("CmpSelectDlg.select_msg");

    RegisterControls();
}

void UCampaignSelectDlg::NativeConstruct()
{
    Super::NativeConstruct();
}

void UCampaignSelectDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    ExecFrame();
}

// +--------------------------------------------------------------------+

void UCampaignSelectDlg::RegisterControls()
{
    // Bind button handlers (AWEvent -> UMG OnClicked):
    if (btn_new)    btn_new->OnClicked.AddDynamic(this, &UCampaignSelectDlg::OnNew);
    if (btn_saved)  btn_saved->OnClicked.AddDynamic(this, &UCampaignSelectDlg::OnSaved);

    if (btn_delete) {
        btn_delete->SetIsEnabled(false);
        btn_delete->OnClicked.AddDynamic(this, &UCampaignSelectDlg::OnDelete);
        // ConfirmDelete is typically driven by a ConfirmDlg callback; keep separate:
        // OnConfirmDelete() will be invoked by your confirm dialog "Yes" action.
    }

    if (btn_accept) {
        btn_accept->SetIsEnabled(false);
        btn_accept->OnClicked.AddDynamic(this, &UCampaignSelectDlg::OnAccept);
    }

    if (btn_cancel) {
        btn_cancel->SetIsEnabled(true);
        btn_cancel->OnClicked.AddDynamic(this, &UCampaignSelectDlg::OnCancel);
    }

    // List selection:
    // UListView selection is UObject-driven. You should bind OnItemSelectionChanged and
    // map it to OnCampaignSelect(). For now, we keep the method and invoke it from your
    // entry widget / list item logic.
    //
    // Example (if you use UObject items):
    // lst_campaigns->OnItemSelectionChanged().AddUObject(this, &UCampaignSelectDlg::HandleSelectionChanged);

    ShowNewCampaigns();
}

// +--------------------------------------------------------------------+

void UCampaignSelectDlg::ExecFrame()
{
    if (Keyboard::KeyDown(VK_RETURN)) {
        if (btn_accept && btn_accept->GetIsEnabled()) {
            OnAccept();
        }
    }

    AutoThreadSync a(sync);

    if (loaded) {
        loaded = false;

        if (btn_cancel)
            btn_cancel->SetIsEnabled(true);

        if (description && btn_accept) {
            if (campaign) {
                Campaign::SelectCampaign(campaign->Name());

                if (load_index >= 0) {
                    // In classic UI, this updated the ListBox image at load_index.
                    // With UListView, images are part of the item object / entry widget.
                    // Keep the image-copy logic for the legacy Bitmap list:
                    if (load_index >= 0 && load_index < images.size()) {
                        images[load_index]->CopyBitmap(*campaign->GetImage(1));
                    }

                    description->SetText(FText::FromString(
                        UTF8_TO_TCHAR(
                            (Text("<font Limerick12><color ffffff>") +
                                campaign->Name() +
                                Text("<font Verdana>\n\n") +
                                Text("<color ffff80>") +
                                Game::GetText("CmpSelectDlg.scenario") +
                                Text("<color ffffff>\n\t") +
                                campaign->Description()).data()
                        )
                    ));
                }
                else {
                    char time_buf[32];
                    char score_buf[32];

                    double t = campaign->GetLoadTime() - campaign->GetStartTime();
                    FormatDayTime(time_buf, t);

                    sprintf_s(score_buf, "%d", campaign->GetPlayerTeamScore());

                    Text desc = Text("<font Limerick12><color ffffff>") +
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

                    desc += Text("\n\n<color ffff80>") +
                        Game::GetText("CmpSelectDlg.team-score") +
                        Text("<color ffffff>\n\t") +
                        score_buf;

                    description->SetText(FText::FromString(UTF8_TO_TCHAR(desc.data())));
                }

                btn_accept->SetIsEnabled(true);

                if (btn_delete)
                    btn_delete->SetIsEnabled(show_saved);
            }
            else {
                description->SetText(FText::FromString(UTF8_TO_TCHAR(select_msg.data())));
                btn_accept->SetIsEnabled(true);
            }
        }
    }
}

bool UCampaignSelectDlg::CanClose()
{
    AutoThreadSync a(sync);
    return !loading;
}

// +--------------------------------------------------------------------+

void UCampaignSelectDlg::ShowNewCampaigns()
{
    AutoThreadSync a(sync);

    if (loading && description) {
        description->SetText(FText::FromString(UTF8_TO_TCHAR(Game::GetText("CmpSelectDlg.already-loading").data())));
        // Button::PlaySound(Button::SND_REJECT); // classic UI sound; hook into your UE sound layer
        return;
    }

    // UMG button visual state is handled by styles; we keep logical intent only.

    if (btn_delete)
        btn_delete->SetIsEnabled(false);

    if (lst_campaigns) {
        images.destroy();

        // UListView population is UObject-driven; you will create item objects for each entry.
        // We keep legacy Bitmap generation here for later entry widgets to reference.

        PlayerCharacter* player = PlayerCharacter::GetCurrentPlayer();
        if (!player)
            return;

        ListIter<Campaign> iter = Campaign::GetAllCampaigns();
        while (++iter) {
            Campaign* c = iter.value();

            if (c->GetCampaignId() < Campaign::SINGLE_MISSIONS) {
                Bitmap* bmp = new Bitmap;
                bmp->CopyBitmap(*c->GetImage(0));
                images.append(bmp);

                // ListView item creation is TODO: create a UObject item holding name + bmp index.
                // Example: UCampaignSelectItem* Item = NewObject<UCampaignSelectItem>(this); ...
                // lst_campaigns->AddItem(Item);

                // FULL GAME CRITERIA (based on player record):
                const int cid = c->GetCampaignId();
                const bool locked_full =
                    (cid > 2 && cid < 10 && !player->HasCompletedCampaign(cid - 1));

                const bool locked_extra =
                    (cid >= 10 && cid < 30 && (cid % 10) != 0 && !player->HasCompletedCampaign(cid - 1));

                if (locked_full || locked_extra) {
                    const int n = images.size() - 1;
                    images[n]->CopyBitmap(*c->GetImage(2));
                }
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

void UCampaignSelectDlg::ShowSavedCampaigns()
{
    AutoThreadSync a(sync);

    if (loading && description) {
        description->SetText(FText::FromString(UTF8_TO_TCHAR(Game::GetText("CmpSelectDlg.already-loading").data())));
        // Button::PlaySound(Button::SND_REJECT);
        return;
    }

    if (btn_delete)
        btn_delete->SetIsEnabled(false);

    if (lst_campaigns) {
        // UListView population is UObject-driven. Build save list here:
        List<Text> save_list;

        CampaignSaveGame::GetSaveGameList(save_list);
        save_list.sort();

        // TODO: create UObjects for each save entry and set as list items.
        // for (int i=0; i<save_list.size(); ++i) { ... }

        save_list.destroy();
    }

    if (description)
        description->SetText(FText::FromString(UTF8_TO_TCHAR(select_msg.data())));

    if (btn_accept)
        btn_accept->SetIsEnabled(false);

    show_saved = true;
}

// +--------------------------------------------------------------------+

void UCampaignSelectDlg::OnCampaignSelect()
{
    if (description && lst_campaigns) {
        AutoThreadSync a(sync);

        if (loading) {
            description->SetText(FText::FromString(UTF8_TO_TCHAR(Game::GetText("CmpSelectDlg.already-loading").data())));
            // Button::PlaySound(Button::SND_REJECT);
            return;
        }

        load_index = -1;
        load_file = "";

        PlayerCharacter* player = PlayerCharacter::GetCurrentPlayer();
        if (!player)
            return;

        // NOTE:
        // In classic, selection came from ListBox indices.
        // In UE, you must fetch the selected UObject item from UListView and map it:
        //
        // UObject* Sel = lst_campaigns->GetSelectedItem();
        // Determine if it's a "new campaign" entry (index) or "save file" entry (Text).
        //
        // For now, preserve behavior by requiring external code to set load_index/load_file
        // prior to calling StartLoadProc().

        if (btn_accept)
            btn_accept->SetIsEnabled(false);
    }

    if (!loading && (load_index >= 0 || load_file.length() > 0)) {
        if (btn_cancel)
            btn_cancel->SetIsEnabled(false);

        StartLoadProc();
    }
}

// +--------------------------------------------------------------------+

void UCampaignSelectDlg::Show()
{
    // In UE, visibility is handled by AddToViewport / SetVisibility.
    // Keep the call surface and refresh.
    ShowNewCampaigns();
}

// +--------------------------------------------------------------------+

void UCampaignSelectDlg::OnNew()
{
    ShowNewCampaigns();
}

void UCampaignSelectDlg::OnSaved()
{
    ShowSavedCampaigns();
}

void UCampaignSelectDlg::OnDelete()
{
    load_file = "";

    // In UE, selection comes from UListView selected item.
    // This port keeps the deletion logic but requires load_file be set from selection mapping.

    if (load_file.length()) {
        // Confirm dialog flow:
        // ConfirmDlg* confirm = manager->GetConfirmDlg();
        // confirm->SetMessage(...); confirm->SetTitle(...); manager->ShowConfirmDlg();
        // Else: OnConfirmDelete();
        UE_LOG(LogTemp, Verbose, TEXT("CampaignSelectDlg: Request delete for save '%s'"), UTF8_TO_TCHAR(load_file.data()));
    }

    ShowSavedCampaigns();
}

void UCampaignSelectDlg::OnConfirmDelete()
{
    if (load_file.length()) {
        CampaignSaveGame::Delete(load_file);
    }

    ShowSavedCampaigns();
}

// +--------------------------------------------------------------------+

void UCampaignSelectDlg::OnAccept()
{
    AutoThreadSync a(sync);

    if (loading)
        return;

    // if this is to be a new campaign, re-instantiate the campaign object
    // NOTE: classic used btn_new->GetButtonState(). In UE, track a bool or enum state.
    // We preserve original intent by using show_saved as a proxy:
    if (!show_saved)
        Campaign::GetCampaign()->Load();
    else
        Game::ResetGameTime();

    Mouse::Show(false);
    if (stars)
        stars->SetGameMode(EGameMode::CLOD);
}

void UCampaignSelectDlg::OnCancel()
{
    // manager->ShowMenuDlg();
    UE_LOG(LogTemp, Verbose, TEXT("CampaignSelectDlg: Cancel -> show menu"));
}

// +--------------------------------------------------------------------+
// Thread proc helpers (ported)
// +--------------------------------------------------------------------+

static uint32 CampaignSelectDlgLoadProc(void* link)
{
    UCampaignSelectDlg* dlg = static_cast<UCampaignSelectDlg*>(link);

    if (dlg)
        return dlg->LoadProc();

    return 0;
}

void UCampaignSelectDlg::StartLoadProc()
{
    // NOTE:
    // This is a minimal, legacy-style thread port. For production UE:
    // - Prefer Async(EAsyncExecution::ThreadPool, ...) or UE::Tasks.
    // - Avoid raw Win32 thread handles in cross-platform builds.

    if (hproc != nullptr) {
        // If you keep a platform thread handle wrapper, check and close it here.
        return;
    }

    campaign = 0;
    loading = true;
    loaded = false;

    if (description)
        description->SetText(FText::FromString(UTF8_TO_TCHAR(Game::GetText("CmpSelectDlg.loading").data())));

    // Placeholder: no raw CreateThread in this port layer.
    // Hook this to UE async when you are ready; for now, run synchronously:
    const uint32 Result = LoadProc();
    UE_LOG(LogTemp, Verbose, TEXT("CampaignSelectDlg: LoadProc result=%u"), Result);
}

void UCampaignSelectDlg::StopLoadProc()
{
    // Placeholder: if you implement an async task/thread, signal stop/join here.
    hproc = nullptr;
}

uint32 UCampaignSelectDlg::LoadProc()
{
    Campaign* c = 0;

    // NEW CAMPAIGN:
    if (load_index >= 0) {
        List<Campaign>& list = Campaign::GetAllCampaigns();

        if (load_index < list.size()) {
            c = list[load_index];
            c->Load();
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

// --------------------------------------------------------------------
// UBaseScreen overrides
// --------------------------------------------------------------------

void UCampaignSelectDlg::BindFormWidgets()
{
    // Map FORM ids to widgets (optional – only if present in your UMG):
    BindButton(100, btn_new);
    BindButton(101, btn_saved);
    BindButton(102, btn_delete);
    BindButton(1, btn_accept);
    BindButton(2, btn_cancel);

    BindList(201, lst_campaigns);
    BindText(200, description);

    BindLabel(10, lbl_title);
    BindLabel(901, lbl_hdr_campaign);
    BindLabel(902, lbl_hdr_desc);

    // Backgrounds are typically Images:
    BindImage(9991, bg_9991);
    BindImage(9992, bg_9992);
}

FString UCampaignSelectDlg::GetLegacyFormText() const
{
    // Keep the original FORM as a raw string, or load it from an asset/datatable later.
    // Returning empty disables auto-application.
    return FString();
}
