/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    ORIGINAL AUTHOR AND STUDIO: John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         MissionSelectDlg.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UMissionSelectDlg (Unreal)
    - Port of MsnSelectDlg
    - Uses UBaseScreen FORM IDs + legacy RichText markup pipeline
    - Uses ComboBoxString when present; ListView support is stubbed until you
      define an item UObject type for UListView.
*/

#include "MissionSelectDlg.h"

#include "Logging/LogMacros.h"

#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/ListView.h"
#include "Components/RichTextBlock.h"
#include "Components/TextBlock.h"

#include "Campaign.h"
#include "Mission.h"
#include "Starshatter.h"
#include "Game.h"
#include "Mouse.h"          // if you still have legacy Mouse::Show in your port
#include "FormatUtil.h"     // FormatDayTime (legacy helper)
#include "GameStructs.h"

// If you are using legacy Text throughout:
#include "Text.h"

DEFINE_LOG_CATEGORY_STATIC(LogMissionSelectDlg, Log, All);

UMissionSelectDlg::UMissionSelectDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    Stars = nullptr;
    CampaignPtr = nullptr;

    SelectedMission = -1;
    MissionId = 0;
    bEditable = false;
}

void UMissionSelectDlg::NativeConstruct()
{
    Super::NativeConstruct();

    // Match legacy: accept disabled until mission selected
    if (UButton* Accept = GetButton(1))
        Accept->SetIsEnabled(false);

    WireEvents();
}

void UMissionSelectDlg::WireEvents()
{
    // Accept / Cancel:
    if (UButton* Accept = GetButton(1))
    {
        if (!Accept->OnClicked.IsAlreadyBound(this, &UMissionSelectDlg::OnAcceptClicked))
            Accept->OnClicked.AddDynamic(this, &UMissionSelectDlg::OnAcceptClicked);
    }

    if (UButton* Cancel = GetButton(2))
    {
        if (!Cancel->OnClicked.IsAlreadyBound(this, &UMissionSelectDlg::OnCancelClicked))
            Cancel->OnClicked.AddDynamic(this, &UMissionSelectDlg::OnCancelClicked);
    }

    // New / Edit / Delete:
    if (UButton* NewBtn = GetButton(301))
    {
        if (!NewBtn->OnClicked.IsAlreadyBound(this, &UMissionSelectDlg::OnNewClicked))
            NewBtn->OnClicked.AddDynamic(this, &UMissionSelectDlg::OnNewClicked);
    }

    if (UButton* EditBtn = GetButton(302))
    {
        if (!EditBtn->OnClicked.IsAlreadyBound(this, &UMissionSelectDlg::OnEditClicked))
            EditBtn->OnClicked.AddDynamic(this, &UMissionSelectDlg::OnEditClicked);
    }

    if (UButton* DelBtn = GetButton(303))
    {
        if (!DelBtn->OnClicked.IsAlreadyBound(this, &UMissionSelectDlg::OnDeleteClicked))
            DelBtn->OnClicked.AddDynamic(this, &UMissionSelectDlg::OnDeleteClicked);
    }

    // Campaign picker:
    // Legacy supported either combo OR list box. In UE, if you use ComboBoxString, we wire it.
    if (UComboBoxString* CampaignCombo = GetCombo(201))
    {
        // NOTE: ComboBoxString uses OnSelectionChanged. Bind in Blueprint or here if you want:
        // CampaignCombo->OnSelectionChanged.AddDynamic(...) is not dynamic; it uses a delegate type
        // that isn't UFUNCTION-friendly in all cases. Easiest: bind in UMG blueprint.
        // For now we rely on manual calls from Show() after setting selection.
    }

    // Mission list:
    // If you're using UListView, you need a UObject item type. Until you provide that, selection
    // handling is intentionally stubbed to compile cleanly.
}

void UMissionSelectDlg::ExecFrame(double DeltaTime)
{
    Super::ExecFrame(DeltaTime);
    (void)DeltaTime;

    // Legacy did VK_RETURN -> OnAccept.
    // In UE you already get Enter/Escape centralized in UBaseScreen::NativeOnKeyDown,
    // so no per-frame polling needed here.
}

void UMissionSelectDlg::Show()
{
    Super::Show();

    Stars = Starshatter::GetInstance();
    CampaignPtr = Campaign::GetCampaign();

    // Campaign selector: prefer combo (201), else list (203)
    UComboBoxString* CmbCampaigns = GetCombo(201);
    UListView* LstCampaigns = GetList(203);
    UListView* LstMissions = GetList(202);
    URichTextBlock* Desc = GetText(200);

    // Clear and repopulate campaign UI (legacy logic)
    if (CmbCampaigns)
    {
        int32 n = 0;
        CmbCampaigns->ClearOptions();

        ListIter<Campaign> iter = Campaign::GetAllCampaigns();
        while (++iter)
        {
            Campaign* C = iter.value();
            if (!C)
                continue;

            if (C->GetCampaignId() >= Campaign::SINGLE_MISSIONS)
            {
                // ComboBoxString requires FString
                CmbCampaigns->AddOption(FString(C->Name()));

                // Legacy auto-select first single mission campaign if current is below SINGLE_MISSIONS:
                if (CampaignPtr && CampaignPtr->GetCampaignId() < Campaign::SINGLE_MISSIONS)
                {
                    CampaignPtr = Campaign::SelectCampaign(C->Name());
                    CmbCampaigns->SetSelectedIndex(n);
                }
                else if (CampaignPtr && CampaignPtr->GetCampaignId() == C->GetCampaignId())
                {
                    CmbCampaigns->SetSelectedIndex(n);
                }

                ++n;
            }
        }
    }
    else if (LstCampaigns)
    {
        // NOTE: UListView needs UObject items. This is a compile-safe stub.
        // Once you define a UObject list item type, populate it here and bind selection.
        LstCampaigns->ClearListItems();
        UE_LOG(LogMissionSelectDlg, Warning, TEXT("MissionSelectDlg: ListView campaigns (203) present, but population requires a UObject item type. Using stub."));
    }

    // Editable flags (legacy: MULTIPLAYER_MISSIONS..CUSTOM_MISSIONS)
    if (CampaignPtr)
    {
        const int32 id = CampaignPtr->GetCampaignId();
        bEditable = (id >= Campaign::MULTIPLAYER_MISSIONS && id <= Campaign::CUSTOM_MISSIONS);

        if (UButton* NewBtn = GetButton(301)) NewBtn->SetIsEnabled(bEditable);
        if (UButton* EditBtn = GetButton(302)) EditBtn->SetIsEnabled(false);
        if (UButton* DelBtn = GetButton(303)) DelBtn->SetIsEnabled(false);
    }

    // Default description
    if (Desc)
    {
        Desc->SetText(FText::FromString(FString(Game::GetText("MsnSelectDlg.choose"))));
    }

    // Missions list:
    if (LstMissions)
    {
        // Compile-safe stub (needs UObject items):
        LstMissions->ClearListItems();
        UE_LOG(LogMissionSelectDlg, Warning, TEXT("MissionSelectDlg: ListView missions (202) present, but population requires a UObject item type. Using stub."));

        // Keep legacy behavior: selection resets, accept disabled
        if (UButton* Accept = GetButton(1))
            Accept->SetIsEnabled(false);

        SelectedMission = -1;
        MissionId = 0;
    }
    else
    {
        // If you switched missions to a ComboBoxString in UMG, support it (optional).
        // Legacy didn't, but UE can.
        // (No action here unless you add a missions combo ID.)
    }
}

// --------------------------------------------------------------------
// Legacy event handlers (UE replacements)
// --------------------------------------------------------------------

void UMissionSelectDlg::OnCampaignSelected()
{
    // If you implement campaign selection via ComboBoxString, call this from BP on selection changed.
    // We keep the legacy logic:
    const char* SelectedCampaign = nullptr;

    if (UComboBoxString* CmbCampaigns = GetCombo(201))
    {
        const FString S = CmbCampaigns->GetSelectedOption();
        if (!S.IsEmpty())
        {
            // Convert to ANSI for legacy APIs expecting const char*
            FTCHARToUTF8 Conv(*S);
            SelectedCampaign = Conv.Get();
        }
    }

    if (!SelectedCampaign)
        return;

    Campaign* C = Campaign::SelectCampaign(SelectedCampaign);
    if (!C)
        return;

    CampaignPtr = C;

    // Disable accept and reset desc
    if (UButton* Accept = GetButton(1))
        Accept->SetIsEnabled(false);

    if (URichTextBlock* Desc = GetText(200))
        Desc->SetText(FText::FromString(FString(Game::GetText("MsnSelectDlg.choose"))));

    // Editable
    const int32 id = C->GetCampaignId();
    bEditable = (id >= Campaign::MULTIPLAYER_MISSIONS && id <= Campaign::CUSTOM_MISSIONS);

    if (UButton* NewBtn = GetButton(301)) NewBtn->SetIsEnabled(bEditable);
    if (UButton* EditBtn = GetButton(302)) EditBtn->SetIsEnabled(false);
    if (UButton* DelBtn = GetButton(303)) DelBtn->SetIsEnabled(false);

    // Rebuild missions list (stub until UListView item type exists)
    if (UListView* LstMissions = GetList(202))
    {
        LstMissions->ClearListItems();
    }

    SelectedMission = -1;
    MissionId = 0;

    // In legacy, this calls OnMissionSelect(0) immediately.
    OnMissionSelected();
}

void UMissionSelectDlg::OnMissionSelected()
{
    // NOTE:
    // You need a real selection model (ComboBoxString or UListView item objects).
    // This method keeps the legacy enable/disable + description formatting ready,
    // but selection retrieval is currently stubbed.

    if (!CampaignPtr)
        return;

    UButton* Accept = GetButton(1);
    URichTextBlock* Desc = GetText(200);

    // Until selection is implemented, force "no selection" state:
    SelectedMission = -1;

    if (SelectedMission < 0)
    {
        if (Desc)
            Desc->SetText(FText::FromString(FString(Game::GetText("MsnSelectDlg.choose"))));

        if (Accept)
            Accept->SetIsEnabled(false);

        if (UButton* EditBtn = GetButton(302)) EditBtn->SetIsEnabled(false);
        if (UButton* DelBtn = GetButton(303)) DelBtn->SetIsEnabled(false);

        return;
    }

    // --- If/when you implement selection, port the legacy body below: ---
    // List<MissionInfo>& list = CampaignPtr->GetMissionList();
    // if (SelectedMission >= 0 && SelectedMission < list.size()) {
    //   MissionInfo* info = list[SelectedMission];
    //   MissionId = info->id;
    //   char time_buf[32]; FormatDayTime(time_buf, info->start);
    //
    //   Text d("<font Limerick12><color ffffff>");
    //   d += info->name;
    //   ...
    //   Desc->SetText(FText::FromString(FString((const char*)d)));
    //   Accept->SetIsEnabled(true);
    //   if (EditBtn) EditBtn->SetIsEnabled(bEditable);
    //   if (DelBtn)  DelBtn->SetIsEnabled(bEditable);
    // }
}

void UMissionSelectDlg::OnAcceptClicked()
{
    if (!CampaignPtr)
        return;

    if (SelectedMission >= 0)
    {
        // Legacy:
        // Mouse::Show(false);
        // int id = campaign->GetMissionList()[selected_mission]->id;
        // campaign->SetMissionId(id);
        // campaign->ReloadMission(id);
        // stars->SetGameMode(Starshatter::PREP_MODE);

        Mouse::Show(false);

        List<MissionInfo>& mission_info_list = CampaignPtr->GetMissionList();

        if (SelectedMission >= 0 && SelectedMission < mission_info_list.size())
        {
            MissionInfo* info = mission_info_list[SelectedMission];
            if (info)
            {
                MissionId = info->id;

                CampaignPtr->SetMissionId(MissionId);
                CampaignPtr->ReloadMission(MissionId);

                if (Stars)
                    Stars->SetGameMode(EMODE::PREP_MODE);
            }
        }
    }
}

void UMissionSelectDlg::OnCancelClicked()
{
    // Legacy: manager->ShowMenuDlg();
    // In UE: your menu manager should own/show/hide this dialog.
    // Keep it simple: just hide; let owner show the menu.
    Hide();
}

void UMissionSelectDlg::OnNewClicked()
{
    // Legacy creates mission and opens editor dialog via manager.
    // Wire later once your MenuScreen equivalent exists.
    UE_LOG(LogMissionSelectDlg, Warning, TEXT("MissionSelectDlg::OnNewClicked - TODO (requires menu manager/editor dialogs)"));
}

void UMissionSelectDlg::OnEditClicked()
{
    UE_LOG(LogMissionSelectDlg, Warning, TEXT("MissionSelectDlg::OnEditClicked - TODO (requires menu manager/editor dialogs)"));
}

void UMissionSelectDlg::OnDeleteClicked()
{
    UE_LOG(LogMissionSelectDlg, Warning, TEXT("MissionSelectDlg::OnDeleteClicked - TODO (confirm dialog + delete flow)"));
}

void UMissionSelectDlg::OnDeleteConfirm()
{
    UE_LOG(LogMissionSelectDlg, Warning, TEXT("MissionSelectDlg::OnDeleteConfirm - TODO"));
}
