/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         MissionBriefingDlg.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UMissionBriefingDlg
    - Unreal UUserWidget replacement for legacy MsnDlg.
    - Inherits from UBaseScreen to use legacy FORM parsing.
    - Implements mission briefing dialog behavior.
*/

#include "MissionBriefingDlg.h"

// Starshatter systems
#include "PlanScreen.h"
#include "Starshatter.h"
#include "Campaign.h"
#include "Mission.h"
#include "Instruction.h"
#include "StarSystem.h"
#include "Game.h"
#include "NetLobby.h"
#include "FormatUtil.h"
#include "Mouse.h"

UMissionBriefingDlg::UMissionBriefingDlg()
{
}

void UMissionBriefingDlg::InitializeMissionBriefing(UPlanScreen* InPlanScreen)
{
    PlanScreen = InPlanScreen;
}

void UMissionBriefingDlg::NativeConstruct()
{
    Super::NativeConstruct();

    // Enable BaseScreen Enter/Escape behavior
    ApplyButton = CommitButton;
    CancelButton = CancelBtn;

    if (CommitButton)
        CommitButton->OnClicked.AddDynamic(this, &UMissionBriefingDlg::OnCommitClicked);

    if (CancelBtn)
        CancelBtn->OnClicked.AddDynamic(this, &UMissionBriefingDlg::OnCancelClicked);

    if (SitButton)
        SitButton->OnClicked.AddDynamic(this, &UMissionBriefingDlg::OnSitClicked);

    if (PkgButton)
        PkgButton->OnClicked.AddDynamic(this, &UMissionBriefingDlg::OnPkgClicked);

    if (NavButton)
        NavButton->OnClicked.AddDynamic(this, &UMissionBriefingDlg::OnNavClicked);

    if (WepButton)
        WepButton->OnClicked.AddDynamic(this, &UMissionBriefingDlg::OnWepClicked);

    ShowMsnDlg();
}

FString UMissionBriefingDlg::GetLegacyFormText() const
{
    return LegacyFormText;
}

void UMissionBriefingDlg::BindFormWidgets()
{
    BindLabel(200, MissionName);
    BindLabel(202, MissionSystem);
    BindLabel(204, MissionSector);
    BindLabel(206, MissionTimeStart);
    BindLabel(208, MissionTimeTarget);
    BindLabel(207, MissionTimeTargetLabel);

    BindButton(900, SitButton);
    BindButton(901, PkgButton);
    BindButton(902, NavButton);
    BindButton(903, WepButton);

    BindButton(1, CommitButton);
    BindButton(2, CancelBtn);
}

void UMissionBriefingDlg::ShowMsnDlg()
{
    campaign = Campaign::GetCampaign();
    mission = nullptr;
    pkg_index = -1;

    if (campaign)
        mission = campaign->GetMission();

    if (MissionName)
    {
        if (mission)
            MissionName->SetText(FText::FromString(ANSI_TO_TCHAR(mission->Name())));
        else
            MissionName->SetText(FText::FromString(ANSI_TO_TCHAR(Game::GetText("MsnDlg.no-mission"))));
    }

    if (MissionSystem)
    {
        MissionSystem->SetText(FText::GetEmpty());
        if (mission && mission->GetStarSystem())
            MissionSystem->SetText(
                FText::FromString(ANSI_TO_TCHAR(mission->GetStarSystem()->Name())));
    }

    if (MissionSector)
    {
        MissionSector->SetText(FText::GetEmpty());
        if (mission)
            MissionSector->SetText(
                FText::FromString(ANSI_TO_TCHAR(mission->GetRegion())));
    }

    if (MissionTimeStart)
    {
        if (mission)
        {
            char txt[32] = { 0 };
            FormatDayTime(txt, mission->Start());
            MissionTimeStart->SetText(FText::FromString(ANSI_TO_TCHAR(txt)));
        }
        else
        {
            MissionTimeStart->SetText(FText::GetEmpty());
        }
    }

    if (MissionTimeTarget)
    {
        int32 time_on_target = CalcTimeOnTarget();

        if (time_on_target)
        {
            char txt[32] = { 0 };
            FormatDayTime(txt, time_on_target);
            MissionTimeTarget->SetText(FText::FromString(ANSI_TO_TCHAR(txt)));

            if (MissionTimeTargetLabel)
                MissionTimeTargetLabel->SetText(
                    FText::FromString(ANSI_TO_TCHAR(Game::GetText("MsnDlg.target"))));
        }
        else
        {
            MissionTimeTarget->SetText(FText::GetEmpty());
            if (MissionTimeTargetLabel)
                MissionTimeTargetLabel->SetText(FText::GetEmpty());
        }
    }

    bool mission_ok = (mission && mission->IsOK());

    if (SitButton) SitButton->SetIsEnabled(mission_ok);
    if (PkgButton) PkgButton->SetIsEnabled(mission_ok);
    if (NavButton) NavButton->SetIsEnabled(mission_ok);

    if (WepButton)
    {
        bool wep_ok = mission_ok;
        if (mission_ok && mission->GetPlayer())
            wep_ok = mission->GetPlayer()->Loadouts().size() > 0;

        if (NetLobby::GetInstance())
            wep_ok = false;

        WepButton->SetIsEnabled(wep_ok);
    }

    if (CommitButton) CommitButton->SetIsEnabled(mission_ok);
    if (CancelBtn)    CancelBtn->SetIsEnabled(true);
}

int32 UMissionBriefingDlg::CalcTimeOnTarget() const
{
    if (!mission)
        return 0;

    MissionElement* element = mission->GetElements()[0];
    if (!element)
        return 0;

    Point loc = element->Location();
    loc.SwapYZ();

    int mission_time = mission->Start();

    ListIter<Instruction> navpt = element->NavList();
    while (++navpt)
    {
        double dist = Point(loc - navpt->Location()).length();
        int etr = (navpt->Speed() > 0) ? (int)(dist / navpt->Speed()) : (int)(dist / 500);

        mission_time += etr;
        loc = navpt->Location();

        if (navpt->Action() >= Instruction::ESCORT)
            return mission_time;
    }

    return 0;
}

// Tab navigation

void UMissionBriefingDlg::OnSitClicked()
{
    if (PlanScreen) PlanScreen->ShowMsnObjDlg();
}

void UMissionBriefingDlg::OnPkgClicked()
{
    if (PlanScreen) PlanScreen->ShowMsnPkgDlg();
}

void UMissionBriefingDlg::OnNavClicked()
{
    if (PlanScreen) PlanScreen->ShowNavDlg();
}

void UMissionBriefingDlg::OnWepClicked()
{
    if (PlanScreen) PlanScreen->ShowMsnWepDlg();
}

// Commit / Cancel

void UMissionBriefingDlg::OnCommitClicked()
{
    Starshatter* stars = Starshatter::GetInstance();
    if (!stars)
        Game::Panic("MissionBriefingDlg::OnCommitClicked - no Starshatter");

    Mouse::Show(false);
    stars->SetGameMode(Starshatter::LOAD_MODE);
}

void UMissionBriefingDlg::OnCancelClicked()
{
    Starshatter* stars = Starshatter::GetInstance();
    if (!stars)
        Game::Panic("MissionBriefingDlg::OnCancelClicked - no Starshatter");

    Mouse::Show(false);

    if (campaign && (campaign->IsDynamic() || campaign->IsTraining()))
        stars->SetGameMode(Starshatter::CMPN_MODE);
    else
        stars->SetGameMode(Starshatter::MENU_MODE);
}
