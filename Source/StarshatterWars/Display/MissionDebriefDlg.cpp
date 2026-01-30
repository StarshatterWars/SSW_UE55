/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         MissionDebriefDlg.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Mission Debriefing dialog (Unreal) - port of DebriefDlg.cpp + DebriefDlg.frm
*/

#include "MissionDebriefDlg.h"
#include "MissionPlanner.h"          // your renamed PlanScreen (do not change MissionPlanner)
#include "Starshatter.h"
#include "Campaign.h"
#include "Mission.h"
#include "MissionInfo.h"
#include "Sim.h"
#include "Ship.h"
#include "StarSystem.h"
#include "Element.h"
#include "Instruction.h"
#include "FormatUtil.h"
#include "Player.h"
#include "SimEvent.h"
#include "ShipDesign.h"

// FORM control wrappers (ported):
#include "Button.h"
#include "ListBox.h"
#include "Text.h"
#include "Keyboard.h"                // if you have a ported keyboard helper
#include "Game.h"                    // legacy Game facade (time compression, text, etc.)

UMissionDebriefDlg::UMissionDebriefDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    CampaignPtr = Campaign::GetCampaign();
    if (CampaignPtr)
    {
        MissionPtr = CampaignPtr->GetMission();
    }
}

void UMissionDebriefDlg::NativeConstruct()
{
    Super::NativeConstruct();

    // FORM-driven screens typically call Init/LoadForm in your UBaseScreen.
    // If your BaseScreen already does that in NativeConstruct, leave it alone.
    RegisterControls();
}

void UMissionDebriefDlg::NativeDestruct()
{
    Super::NativeDestruct();
}

void UMissionDebriefDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (bIsShown)
    {
        ExecFrame(InDeltaTime);
    }
}

// +--------------------------------------------------------------------+
// Control binding
// +--------------------------------------------------------------------+

void UMissionDebriefDlg::RegisterControls()
{
    // Header/info controls:
    MissionName = (UActiveWindow*)FindControl(200);
    MissionSystem = (UActiveWindow*)FindControl(202);
    MissionSector = (UActiveWindow*)FindControl(204);
    MissionTimeStart = (UActiveWindow*)FindControl(206);

    Objectives = (UActiveWindow*)FindControl(210);
    Situation = (UActiveWindow*)FindControl(240);
    MissionScore = (UActiveWindow*)FindControl(211);

    UnitList = (UListBox*)FindControl(320);
    SummaryList = (UListBox*)FindControl(330);
    EventList = (UListBox*)FindControl(340);

    // Button:
    CloseButton = (UButton*)FindControl(1);

    // Wire events using your wrapper’s delegate system.
    // (Names below match typical ports of the legacy UI; adjust if your wrappers differ.)

    if (UnitList)
    {
        // legacy: REGISTER_CLIENT(EID_SELECT, unit_list, DebriefDlg, OnUnit);
        UnitList->OnSelectionChanged().AddUObject(this, &UMissionDebriefDlg::OnUnit);
    }

    if (CloseButton)
    {
        // legacy: REGISTER_CLIENT(EID_CLICK, close_btn, DebriefDlg, OnClose);
        CloseButton->OnClicked().AddUObject(this, &UMissionDebriefDlg::OnClose);
    }
}

// +--------------------------------------------------------------------+
// Show/Hide
// +--------------------------------------------------------------------+

void UMissionDebriefDlg::Show()
{
    Super::Show();
    bIsShown = true;

    // Legacy behavior:
    Game::SetTimeCompression(1);

    MissionPtr = nullptr;
    CampaignPtr = Campaign::GetCampaign();
    SimPtr = Sim::GetSim();

    if (SimPtr)
    {
        PlayerShip = SimPtr->GetPlayerShip();
    }

    if (CampaignPtr)
    {
        MissionPtr = CampaignPtr->GetMission();
    }

    // Mission title:
    if (MissionName)
    {
        if (MissionPtr)
            MissionName->SetText(MissionPtr->Name());
        else
            MissionName->SetText(Game::GetText("DebriefDlg.mission-name"));
    }

    // System:
    if (MissionSystem)
    {
        MissionSystem->SetText("");

        if (MissionPtr)
        {
            StarSystem* Sys = MissionPtr->GetStarSystem();
            if (Sys)
                MissionSystem->SetText(Sys->Name());
        }
    }

    // Sector:
    if (MissionSector)
    {
        MissionSector->SetText("");

        if (MissionPtr)
        {
            // legacy uses mission->GetElements()[0] to get region/sector
            MissionElement* Elem0 = MissionPtr->GetElements().size() ? MissionPtr->GetElements().at(0) : nullptr;
            if (Elem0)
                MissionSector->SetText(Elem0->Region());
        }
    }

    // Start time:
    if (MissionTimeStart)
    {
        if (MissionPtr)
        {
            char Txt[32] = { 0 };
            FormatDayTime(Txt, MissionPtr->Start());
            MissionTimeStart->SetText(Txt);
        }
    }

    // Objectives:
    if (Objectives)
    {
        bool bFoundObjectives = false;

        if (SimPtr && SimPtr->GetPlayerElement())
        {
            Text T;
            Element* PlayerElem = SimPtr->GetPlayerElement();

            for (int i = 0; i < PlayerElem->NumObjectives(); i++)
            {
                Instruction* Obj = PlayerElem->GetObjective(i);
                T += Text("* ") + Obj->GetDescription() + Text("\n");
                bFoundObjectives = true;
            }

            Objectives->SetText(T);
        }

        if (!bFoundObjectives)
        {
            if (MissionPtr)
                Objectives->SetText(MissionPtr->Objective());
            else
                Objectives->SetText(Game::GetText("DebriefDlg.unspecified"));
        }
    }

    // Situation:
    if (Situation)
    {
        if (MissionPtr)
            Situation->SetText(MissionPtr->Situation());
        else
            Situation->SetText(Game::GetText("DebriefDlg.unknown"));
    }

    // Score:
    if (MissionScore)
    {
        MissionScore->SetText(Game::GetText("DebriefDlg.no-stats"));

        if (PlayerShip)
        {
            for (int i = 0; i < ShipStats::NumStats(); i++)
            {
                ShipStats* Stats = ShipStats::GetStats(i);
                if (Stats && !strcmp(PlayerShip->Name(), Stats->GetName()))
                {
                    Stats->Summarize();

                    Player* PlayerObj = Player::GetCurrentPlayer();
                    int Points = Stats->GetPoints() + Stats->GetCommandPoints();

                    if (PlayerObj && SimPtr)
                        Points = PlayerObj->GetMissionPoints(Stats, SimPtr->StartTime()) + Stats->GetCommandPoints();

                    char Score[32] = { 0 };
                    sprintf_s(Score, "%d %s", Points, Game::GetText("DebriefDlg.points").data());
                    MissionScore->SetText(Score);
                    break;
                }
            }
        }
    }

    DrawUnits();
}

void UMissionDebriefDlg::Hide()
{
    bIsShown = false;
    Super::Hide();
}

// +--------------------------------------------------------------------+
// Units list + selection
// +--------------------------------------------------------------------+

void UMissionDebriefDlg::DrawUnits()
{
    if (!MissionPtr || !UnitList)
        return;

    UnitList->ClearItems();

    int SelIndex = -1;
    bool bNetGame = false;

    if (SimPtr && SimPtr->IsNetGame())
        bNetGame = true;

    for (int i = 0; i < ShipStats::NumStats(); i++)
    {
        ShipStats* Stats = ShipStats::GetStats(i);
        if (!Stats) continue;

        Stats->Summarize();

        if (bNetGame ||
            (Stats->GetIFF() == MissionPtr->Team() &&
                !strcmp(Stats->GetRegion(), MissionPtr->GetRegion())))
        {
            int Row = UnitList->AddItemWithData(" ", i) - 1;
            UnitList->SetItemText(Row, 1, Stats->GetName());
            UnitList->SetItemText(Row, 2, Stats->GetRole());
            UnitList->SetItemText(Row, 3, Stats->GetType());

            if (PlayerShip && !strcmp(PlayerShip->Name(), Stats->GetName()))
                SelIndex = Row;
        }
    }

    if (SelIndex >= 0)
    {
        UnitList->SetSelected(SelIndex);
        OnUnit();
    }
}

// +--------------------------------------------------------------------+
// Frame
// +--------------------------------------------------------------------+

void UMissionDebriefDlg::ExecFrame(float DeltaSeconds)
{
    // Legacy: if list has items but none selected, force select first
    if (UnitList && UnitList->NumItems() && UnitList->GetSelCount() < 1)
    {
        UnitList->SetSelected(0);
        OnUnit();
    }

    // Legacy: Enter closes
    if (Keyboard::KeyDown(VK_RETURN))
    {
        OnClose();
    }
}

// +--------------------------------------------------------------------+
// Event handlers
// +--------------------------------------------------------------------+

void UMissionDebriefDlg::OnUnit()
{
    if (!UnitList || !EventList || !SummaryList)
        return;

    SummaryList->ClearItems();
    EventList->ClearItems();

    const int Sel = UnitList->GetSelection();
    const int Unit = UnitList->GetItemData(Sel);

    ShipStats* Stats = ShipStats::GetStats(Unit);
    if (!Stats)
        return;

    Stats->Summarize();

    char Txt[64] = { 0 };
    int Row = 0;

    sprintf_s(Txt, "%d", Stats->GetGunShots());
    SummaryList->AddItem("Guns Fired: ");
    SummaryList->SetItemText(Row++, 1, Txt);

    sprintf_s(Txt, "%d", Stats->GetGunHits());
    SummaryList->AddItem("Gun Hits: ");
    SummaryList->SetItemText(Row++, 1, Txt);

    sprintf_s(Txt, "%d", Stats->GetGunKills());
    SummaryList->AddItem("Gun Kills: ");
    SummaryList->SetItemText(Row++, 1, Txt);

    // spacer
    SummaryList->AddItem(" ");
    Row++;

    sprintf_s(Txt, "%d", Stats->GetMissileShots());
    SummaryList->AddItem("Missiles Fired: ");
    SummaryList->SetItemText(Row++, 1, Txt);

    sprintf_s(Txt, "%d", Stats->GetMissileHits());
    SummaryList->AddItem("Missile Hits: ");
    SummaryList->SetItemText(Row++, 1, Txt);

    sprintf_s(Txt, "%d", Stats->GetMissileKills());
    SummaryList->AddItem("Missile Kills: ");
    SummaryList->SetItemText(Row++, 1, Txt);

    // Events:
    int EventRow = 0;
    ListIter<SimEvent> Iter = Stats->GetEvents();
    while (++Iter)
    {
        SimEvent* E = Iter.value();
        if (!E) continue;

        char TimeTxt[64] = { 0 };
        const int Time = E->GetTime();

        if (Time > 24 * 60 * 60)
            FormatDayTime(TimeTxt, Time);
        else
            FormatTime(TimeTxt, Time);

        EventList->AddItem(TimeTxt);
        EventList->SetItemText(EventRow, 1, E->GetEventDesc());

        if (E->GetTarget())
            EventList->SetItemText(EventRow, 2, E->GetTarget());

        EventRow++;
    }
}

void UMissionDebriefDlg::OnClose()
{
    Sim* LocalSim = Sim::GetSim();
    if (!LocalSim)
        return;

    LocalSim->CommitMission();
    LocalSim->UnloadMission();

    // Legacy net host flow (keep if you still support it):
    NetLobby* Lobby = NetLobby::GetInstance();
    if (Lobby && Lobby->IsHost())
    {
        Lobby->SelectMission(0);
        Lobby->ExecFrame();
    }

    Player* PlayerObj = Player::GetCurrentPlayer();
    if (PlayerObj && PlayerObj->ShowAward())
    {
        if (Manager)
        {
            Manager->ShowAwardDlg();
        }
        return;
    }

    // Back to campaign/menu like legacy:
    Starshatter* Stars = Starshatter::GetInstance();
    if (Stars)
    {
        Mouse::Show(false);

        Campaign* Camp = Campaign::GetCampaign();
        if (Camp && Camp->GetCampaignId() < Campaign::SINGLE_MISSIONS)
            Stars->SetGameMode(Starshatter::CMPN_MODE);
        else
            Stars->SetGameMode(Starshatter::MENU_MODE);
    }
    else
    {
        Game::Panic("MissionDebriefDlg::OnClose() - Game instance not found");
    }
}
