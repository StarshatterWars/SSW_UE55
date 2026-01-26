/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         CmdOrdersDlg.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UCmdOrdersDlg implementation (Unreal port)
*/

#include "CmdOrdersDlg.h"

// UMG
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"

// Starshatter core
#include "Starshatter.h"
#include "Campaign.h"
#include "CombatGroup.h"
#include "Game.h"
#include "FormatUtil.h"
#include "Mouse.h"

// Your screen manager
#include "CmpnScreen.h"

UCmdOrdersDlg::UCmdOrdersDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UCmdOrdersDlg::NativeConstruct()
{
    Super::NativeConstruct();

    Stars = Starshatter::GetInstance();
    CampaignPtr = Campaign::GetCampaign();

    // Bind buttons
    if (btn_save)     btn_save->OnClicked.AddDynamic(this, &UCmdOrdersDlg::OnSaveClicked);
    if (btn_exit)     btn_exit->OnClicked.AddDynamic(this, &UCmdOrdersDlg::OnExitClicked);

    if (btn_orders)   btn_orders->OnClicked.AddDynamic(this, &UCmdOrdersDlg::OnModeOrdersClicked);
    if (btn_theater)  btn_theater->OnClicked.AddDynamic(this, &UCmdOrdersDlg::OnModeTheaterClicked);
    if (btn_forces)   btn_forces->OnClicked.AddDynamic(this, &UCmdOrdersDlg::OnModeForcesClicked);
    if (btn_intel)    btn_intel->OnClicked.AddDynamic(this, &UCmdOrdersDlg::OnModeIntelClicked);
    if (btn_missions) btn_missions->OnClicked.AddDynamic(this, &UCmdOrdersDlg::OnModeMissionsClicked);
}

void UCmdOrdersDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    ExecFrame();
}

void UCmdOrdersDlg::SetManager(UCmpnScreen* InManager)
{
    Manager = InManager;
}

void UCmdOrdersDlg::ShowOrdersDlg()
{
    // Mirrors legacy: mode = MODE_ORDERS, FormWindow::Show(), ShowCmdDlg()
    // In Unreal: just make visible and populate.
    Mode = 0; // If you have UCmdDlg::ECmdMode, replace with MODE_ORDERS.

    CampaignPtr = Campaign::GetCampaign();

    // Title / campaign name
    if (txt_name)
    {
        if (CampaignPtr)
            txt_name->SetText(FText::FromString(UTF8_TO_TCHAR(CampaignPtr->Name())));
        else
            txt_name->SetText(FText::FromString(TEXT("No Campaign Selected")));
    }

    // Populate the orders body (legacy built rich text with font tags)
    if (CampaignPtr)
    {
        FString OrdersText;

        // Legacy:
        // <font Limerick12><color ffff80> situation header
        // <font Verdana><color ffffff> situation text
        // <font Limerick12><color ffff80> orders header
        // <font Verdana><color ffffff> orders text
        const char* SituationHdr = Game::GetText("CmdOrdersDlg.situation");
        const char* OrdersHdr = Game::GetText("CmdOrdersDlg.orders");

        const char* Situation = CampaignPtr->Situation();
        const char* Desc = CampaignPtr->Description();
        const char* Orders = CampaignPtr->Orders();

        // If you use URichTextBlock with a decorator/style set, keep markup minimal and ASCII-only.
        // Otherwise, fall back to plain text.
        OrdersText += FString::Printf(TEXT("%s\n\n"), UTF8_TO_TCHAR(SituationHdr));

        if (Situation && *Situation)
            OrdersText += UTF8_TO_TCHAR(Situation);
        else if (Desc && *Desc)
            OrdersText += UTF8_TO_TCHAR(Desc);

        OrdersText += TEXT("\n\n");
        OrdersText += FString::Printf(TEXT("%s\n\n"), UTF8_TO_TCHAR(OrdersHdr));

        if (Orders && *Orders)
            OrdersText += UTF8_TO_TCHAR(Orders);

        if (txt_orders_rich)
        {
            txt_orders_rich->SetText(FText::FromString(OrdersText));
        }
        else if (txt_orders)
        {
            txt_orders->SetText(FText::FromString(OrdersText));
        }
    }
    else
    {
        if (txt_orders_rich)
            txt_orders_rich->SetText(FText::FromString(TEXT("")));
        else if (txt_orders)
            txt_orders->SetText(FText::FromString(TEXT("")));
    }

    SetVisibility(ESlateVisibility::Visible);
}

void UCmdOrdersDlg::ExecFrame()
{
    // Mirrors CmdDlg::ExecFrame header updates (same pattern you used in UCmdForceDlg)
    if (!CampaignPtr)
        CampaignPtr = Campaign::GetCampaign();

    if (!CampaignPtr)
        return;

    // Player group label
    if (txt_group)
    {
        CombatGroup* G = CampaignPtr->GetPlayerGroup();
        if (G)
            txt_group->SetText(FText::FromString(UTF8_TO_TCHAR(G->GetDescription())));
    }

    // Team score label (right-justified)
    if (txt_score)
    {
        const int32 TeamScore = CampaignPtr->GetPlayerTeamScore();
        const FString ScoreStr = FString::Printf(TEXT("Team Score: %d"), TeamScore);
        txt_score->SetText(FText::FromString(ScoreStr));
        txt_score->SetJustification(ETextJustify::Right);
    }

    // Day/time label
    if (txt_time)
    {
        char DayTime[32] = { 0 };
        FormatDayTime(DayTime, CampaignPtr->GetTime());
        txt_time->SetText(FText::FromString(UTF8_TO_TCHAR(DayTime)));
    }
}

void UCmdOrdersDlg::SetModeAndHighlight(int32 InMode)
{
    Mode = InMode;

    if (!Manager)
    {
        UE_LOG(LogTemp, Warning, TEXT("CmdOrdersDlg: Manager is null (SetModeAndHighlight)."));
        return;
    }

    // Replace these calls with your UCmpnScreen routing functions
    // (keeping ASCII-only, no smart quotes).
    switch (Mode)
    {
    case 0: Manager->ShowCmdOrdersDlg();   break; // MODE_ORDERS
    case 1: Manager->ShowCmdTheaterDlg();  break; // MODE_THEATER
    case 2: Manager->ShowCmdForceDlg();    break; // MODE_FORCES
    case 3: Manager->ShowCmdIntelDlg();    break; // MODE_INTEL
    case 4: Manager->ShowCmdMissionsDlg(); break; // MODE_MISSIONS
    default: Manager->ShowCmdOrdersDlg();  break;
    }
}

void UCmdOrdersDlg::OnModeOrdersClicked() { SetModeAndHighlight(0); }
void UCmdOrdersDlg::OnModeTheaterClicked() { SetModeAndHighlight(1); }
void UCmdOrdersDlg::OnModeForcesClicked() { SetModeAndHighlight(2); }
void UCmdOrdersDlg::OnModeIntelClicked() { SetModeAndHighlight(3); }
void UCmdOrdersDlg::OnModeMissionsClicked() { SetModeAndHighlight(4); }

void UCmdOrdersDlg::OnSaveClicked()
{
    if (Manager)
        Manager->ShowCmpFileDlg();
    else
        UE_LOG(LogTemp, Warning, TEXT("CmdOrdersDlg: Manager is null (OnSaveClicked)."));
}

void UCmdOrdersDlg::OnExitClicked()
{
    if (Stars)
    {
        Mouse::Show(false);
        Stars->SetGameMode((int)EMODE::MENU_MODE);
    }
}
