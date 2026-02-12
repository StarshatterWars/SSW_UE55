/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         CmdMissionsDlg.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UCmdMissionsDlg implementation (Unreal port)

    NOTES (Unreal Port)
    ===================
    - Legacy PlayerCharacter training flags have been migrated to UStarshatterPlayerSubsystem:
        UStarshatterPlayerSubsystem::HasTrainedSafe(WorldCtx, MissionId)
    - Therefore, this dialog does NOT depend on PlayerCharacter for training completion.
*/

#include "CmdMissionsDlg.h"

// UMG
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ListView.h"
#include "Components/RichTextBlock.h"

// Starshatter core
#include "Starshatter.h"
#include "Campaign.h"
#include "Mission.h"
#include "Game.h"
#include "Mouse.h"
#include "FormatUtil.h"
#include "CombatGroup.h"
#include "GameStructs.h" // (Fix casing; your paste had GameSTructs.h)

// Your campaign screen
#include "CmpnScreen.h"

// Player profile persistence (training bits)
#include "StarshatterPlayerSubsystem.h"

UCmdMissionsDlg::UCmdMissionsDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UCmdMissionsDlg::NativeConstruct()
{
    Super::NativeConstruct();

    BindFormWidgets();

    Stars = Starshatter::GetInstance();
    CampaignPtr = Campaign::GetCampaign();

    // Buttons
    if (btn_save)     btn_save->OnClicked.AddDynamic(this, &UCmdMissionsDlg::OnSaveClicked);
    if (btn_exit)     btn_exit->OnClicked.AddDynamic(this, &UCmdMissionsDlg::OnExitClicked);

    if (btn_orders)   btn_orders->OnClicked.AddDynamic(this, &UCmdMissionsDlg::OnModeOrdersClicked);
    if (btn_theater)  btn_theater->OnClicked.AddDynamic(this, &UCmdMissionsDlg::OnModeTheaterClicked);
    if (btn_forces)   btn_forces->OnClicked.AddDynamic(this, &UCmdMissionsDlg::OnModeForcesClicked);
    if (btn_intel)    btn_intel->OnClicked.AddDynamic(this, &UCmdMissionsDlg::OnModeIntelClicked);
    if (btn_missions) btn_missions->OnClicked.AddDynamic(this, &UCmdMissionsDlg::OnModeMissionsClicked);

    if (btn_accept)   btn_accept->OnClicked.AddDynamic(this, &UCmdMissionsDlg::OnAcceptClicked);

    if (lst_missions)
        lst_missions->OnItemClicked().AddUObject(this, &UCmdMissionsDlg::OnMissionItemClicked);

    if (btn_accept)
        btn_accept->SetIsEnabled(false);

    ClearDescription();
}

void UCmdMissionsDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    ExecFrame();
}

void UCmdMissionsDlg::SetManager(UCmpnScreen* InManager)
{
    Manager = InManager;
}

void UCmdMissionsDlg::BindFormWidgets()
{
    // Intentionally empty if you are using BindWidgetOptional.
    // If you are using legacy FRM binding, bind here.
}

void UCmdMissionsDlg::ShowMissionsDlg()
{
    Mode = ECOMMAND_MODE::MODE_MISSIONS;

    CampaignPtr = Campaign::GetCampaign();
    Stars = Starshatter::GetInstance();

    if (txt_name)
    {
        if (CampaignPtr)
            txt_name->SetText(FText::FromString(CampaignPtr->Name()));
        else
            txt_name->SetText(FText::FromString(TEXT("No Campaign Selected")));
    }

    RebuildMissionList();
    UpdateAcceptEnabled();

    SetVisibility(ESlateVisibility::Visible);
}

void UCmdMissionsDlg::ExecFrame()
{
    ExecHeaderFrame();

    if (!CampaignPtr)
        CampaignPtr = Campaign::GetCampaign();

    if (!CampaignPtr || !lst_missions)
        return;

    // Legacy behavior:
    // - Append if missions grew
    // - Rebuild if missions shrank
    // - Rebuild if mission IDs changed / first id invalid
    AppendNewMissionsIfAny();
    ValidateSelectionStillExists();
    UpdateAcceptEnabled();
}

void UCmdMissionsDlg::ExecHeaderFrame()
{
    if (!CampaignPtr)
        CampaignPtr = Campaign::GetCampaign();

    if (!CampaignPtr)
        return;

    if (txt_group)
    {
        CombatGroup* G = CampaignPtr->GetPlayerGroup();
        if (G)
            txt_group->SetText(FText::FromString(UTF8_TO_TCHAR(G->GetDescription())));
    }

    if (txt_score)
    {
        const int32 TeamScore = CampaignPtr->GetPlayerTeamScore();
        txt_score->SetText(FText::FromString(FString::Printf(TEXT("Team Score: %d"), TeamScore)));
    }

    if (txt_time)
    {
        char DayTime[32] = { 0 };
        FormatDayTime(DayTime, CampaignPtr->GetTime());
        txt_time->SetText(FText::FromString(UTF8_TO_TCHAR(DayTime)));
    }
}

void UCmdMissionsDlg::RebuildMissionList()
{
    if (!CampaignPtr || !lst_missions)
        return;

    lst_missions->ClearListItems();
    ClearDescription();
    SelectedMission = nullptr;

    List<MissionInfo>& Missions = CampaignPtr->GetMissionList();

    for (int32 i = 0; i < Missions.size(); ++i)
    {
        MissionInfo* Info = Missions[i];
        if (!Info) continue;

        UCmdMissionListItem* Item = NewObject<UCmdMissionListItem>(this);
        Item->MissionName = UTF8_TO_TCHAR(Info->name);
        Item->MissionId = Info->id;

        Mission* M = Info->mission;
        Item->MissionPtr = M;

        // Type column: if training and already trained, show "training" localized string
        if (M)
        {
            bool bTrained = false;

            if (M->Type() == Mission::TRAINING)
            {
                // Training completion migrated to PlayerSubsystem (bitmask)
                bTrained = UStarshatterPlayerSubsystem::HasTrainedSafe(this, M->Identity());
            }

            if (M->Type() == Mission::TRAINING && bTrained)
                Item->MissionType = UTF8_TO_TCHAR(Game::GetText("CmdMissionsDlg.training"));
            else
                Item->MissionType = UTF8_TO_TCHAR(M->TypeName());
        }

        // Start column
        char StartTime[64] = { 0 };
        FormatDayTime(StartTime, Info->start);
        Item->StartTime = UTF8_TO_TCHAR(StartTime);

        lst_missions->AddItem(Item);
    }
}

void UCmdMissionsDlg::AppendNewMissionsIfAny()
{
    if (!CampaignPtr || !lst_missions)
        return;

    List<MissionInfo>& Missions = CampaignPtr->GetMissionList();
    const int32 Existing = lst_missions->GetNumItems();
    const int32 Total = Missions.size();

    if (Total < Existing)
    {
        // Legacy: clear and rebuild when list shrinks
        const int32 PrevSelectedId = GetSelectedMissionId();
        RebuildMissionList();
        if (PrevSelectedId > 0)
            SetSelectedMissionId(PrevSelectedId);
        return;
    }

    if (Total == Existing)
    {
        // Legacy had additional validation (first id must still exist):
        if (Total > 0 && Existing > 0)
        {
            UObject* FirstObj = lst_missions->GetItemAt(0);
            UCmdMissionListItem* FirstItem = Cast<UCmdMissionListItem>(FirstObj);
            const int32 FirstId = FirstItem ? FirstItem->MissionId : 0;

            MissionInfo* InfoCheck = (FirstId > 0) ? CampaignPtr->GetMissionInfo(FirstId) : nullptr;
            if (!InfoCheck)
            {
                const int32 PrevSelectedId = GetSelectedMissionId();
                RebuildMissionList();
                if (PrevSelectedId > 0)
                    SetSelectedMissionId(PrevSelectedId);
            }
        }

        return;
    }

    // Append new missions
    List<MissionInfo>& ListRef = CampaignPtr->GetMissionList();

    for (int32 i = Existing; i < Total; ++i)
    {
        MissionInfo* Info = ListRef[i];
        if (!Info) continue;

        UCmdMissionListItem* Item = NewObject<UCmdMissionListItem>(this);
        Item->MissionName = UTF8_TO_TCHAR(Info->name);
        Item->MissionId = Info->id;
        Item->MissionPtr = Info->mission;

        Mission* M = Info->mission;
        if (M)
        {
            if (M->Type() == Mission::TRAINING &&
                UStarshatterPlayerSubsystem::HasTrainedSafe(this, M->Identity()))
            {
                Item->MissionType = UTF8_TO_TCHAR(Game::GetText("CmdMissionsDlg.training"));
            }
            else
            {
                Item->MissionType = UTF8_TO_TCHAR(M->TypeName());
            }
        }

        char StartTime[64] = { 0 };
        FormatDayTime(StartTime, Info->start);
        Item->StartTime = UTF8_TO_TCHAR(StartTime);

        lst_missions->AddItem(Item);
    }
}

void UCmdMissionsDlg::ValidateSelectionStillExists()
{
    if (!CampaignPtr)
        return;

    // Legacy: if mission pointer not in list anymore, clear selection/desc
    List<MissionInfo>& Missions = CampaignPtr->GetMissionList();

    bool bFound = false;
    for (int32 i = 0; i < Missions.size(); ++i)
    {
        MissionInfo* Info = Missions[i];
        if (Info && Info->mission == SelectedMission)
        {
            bFound = true;
            break;
        }
    }

    if (!bFound)
    {
        SelectedMission = nullptr;
        ClearDescription();
        if (btn_accept)
            btn_accept->SetIsEnabled(false);
    }
}

int32 UCmdMissionsDlg::GetSelectedMissionId() const
{
    if (!lst_missions)
        return 0;

    UObject* Selected = lst_missions->GetSelectedItem();
    UCmdMissionListItem* Item = Cast<UCmdMissionListItem>(Selected);
    return Item ? Item->MissionId : 0;
}

void UCmdMissionsDlg::SetSelectedMissionId(int32 MissionId)
{
    if (!lst_missions || MissionId <= 0)
        return;

    const int32 Num = lst_missions->GetNumItems();
    for (int32 i = 0; i < Num; ++i)
    {
        UObject* Obj = lst_missions->GetItemAt(i);
        UCmdMissionListItem* Item = Cast<UCmdMissionListItem>(Obj);
        if (Item && Item->MissionId == MissionId)
        {
            lst_missions->SetSelectedItem(Obj);
            break;
        }
    }
}

void UCmdMissionsDlg::OnMissionItemClicked(UObject* ItemObj)
{
    if (!CampaignPtr || !lst_missions)
        return;

    lst_missions->SetSelectedItem(ItemObj);

    UCmdMissionListItem* Item = Cast<UCmdMissionListItem>(ItemObj);
    if (!Item)
    {
        SelectedMission = nullptr;
        ClearDescription();
        UpdateAcceptEnabled();
        return;
    }

    MissionInfo* Info = CampaignPtr->GetMissionInfo(Item->MissionId);

    if (Info)
    {
        SetDescriptionForMissionInfo(Info);
        SelectedMission = Info->mission;
    }
    else
    {
        SelectedMission = nullptr;
        ClearDescription();
    }

    UpdateAcceptEnabled();
}

void UCmdMissionsDlg::SetDescriptionForMissionInfo(MissionInfo* Info)
{
    if (!txt_desc || !Info)
        return;

    FString Desc;

    // Legacy markup: <font Limerick12><color ffff80>title<font Verdana><color ffffff>
    // In UMG RichTextBlock, you will need matching styles; otherwise use plain text.
    Desc += TEXT("<Title>");
    Desc += UTF8_TO_TCHAR(Info->name);
    Desc += TEXT("</>\n\n");
    Desc += UTF8_TO_TCHAR(Info->player_info);
    Desc += TEXT("\n\n");
    Desc += UTF8_TO_TCHAR(Info->description);

    txt_desc->SetText(FText::FromString(Desc));
}

void UCmdMissionsDlg::ClearDescription()
{
    if (txt_desc)
        txt_desc->SetText(FText::GetEmpty());
}

bool UCmdMissionsDlg::CanAcceptMission(MissionInfo* Info) const
{
    return (Info != nullptr);
}

void UCmdMissionsDlg::UpdateAcceptEnabled()
{
    if (!btn_accept || !CampaignPtr)
        return;

    const int32 SelectedId = GetSelectedMissionId();
    MissionInfo* Info = (SelectedId > 0) ? CampaignPtr->GetMissionInfo(SelectedId) : nullptr;

    btn_accept->SetIsEnabled(CanAcceptMission(Info));
}

void UCmdMissionsDlg::OnAcceptClicked()
{
    if (!CampaignPtr || !SelectedMission || !Stars)
        return;

    // Legacy behavior
    Mouse::Show(false);
    CampaignPtr->SetMissionId(SelectedMission->Identity());
    CampaignPtr->StartMission();
    Stars->SetGameMode(EGameMode::PREP);
}

// --------------------------------------------------------------------
// Routing
// --------------------------------------------------------------------

void UCmdMissionsDlg::SetModeAndRoute(ECOMMAND_MODE InMode)
{
    Mode = InMode;

    if (!Manager)
    {
        UE_LOG(LogTemp, Warning, TEXT("CmdMissionsDlg: Manager is null (SetModeAndRoute)."));
        return;
    }

    switch (Mode)
    {
    case ECOMMAND_MODE::MODE_ORDERS:   Manager->ShowCmdOrdersDlg();   break;
    case ECOMMAND_MODE::MODE_THEATER:  Manager->ShowCmdTheaterDlg();  break;
    case ECOMMAND_MODE::MODE_FORCES:   Manager->ShowCmdForceDlg();    break;
    case ECOMMAND_MODE::MODE_INTEL:    Manager->ShowCmdIntelDlg();    break;
    case ECOMMAND_MODE::MODE_MISSIONS: Manager->ShowCmdMissionsDlg(); break;
    default:                           Manager->ShowCmdOrdersDlg();   break;
    }
}

void UCmdMissionsDlg::OnModeOrdersClicked() { SetModeAndRoute(ECOMMAND_MODE::MODE_ORDERS); }
void UCmdMissionsDlg::OnModeTheaterClicked() { SetModeAndRoute(ECOMMAND_MODE::MODE_THEATER); }
void UCmdMissionsDlg::OnModeForcesClicked() { SetModeAndRoute(ECOMMAND_MODE::MODE_FORCES); }
void UCmdMissionsDlg::OnModeIntelClicked() { SetModeAndRoute(ECOMMAND_MODE::MODE_INTEL); }
void UCmdMissionsDlg::OnModeMissionsClicked() { SetModeAndRoute(ECOMMAND_MODE::MODE_MISSIONS); }

void UCmdMissionsDlg::OnSaveClicked()
{
    if (Manager)
        Manager->ShowCmpFileDlg();
}

void UCmdMissionsDlg::OnExitClicked()
{
    if (Stars)
    {
        Mouse::Show(false);
        Stars->SetGameMode(EGameMode::MENU);
    }
}
