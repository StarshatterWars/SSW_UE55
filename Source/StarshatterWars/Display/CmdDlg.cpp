/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         CmdDlg.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UCmdDlg implementation (Unreal port)
*/

#include "CmdDlg.h"

// UMG:
#include "Components/TextBlock.h"
#include "Components/Button.h"

// Starshatter core:
#include "Campaign.h"
#include "CombatGroup.h"
#include "Starshatter.h"
#include "FormatUtil.h"      // FormatDayTime(...)
#include "Mouse.h"           // Mouse::Show(...)

// Your campaign screen widget (port of CmpnScreen):
#include "CmpnScreen.h"
// Your campaign file dialog widget (port of CmpFileDlg):
#include "CmpFileDlg.h"

#include "GameStructs.h"

UCmdDlg::UCmdDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UCmdDlg::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind FORM IDs to widgets:
    BindFormWidgets();

    // Bind click handlers (use AddDynamic; no AddLambda on OnClicked in UE UButton)
    if (btn_save)     btn_save->OnClicked.AddDynamic(this, &UCmdDlg::OnSaveClicked);
    if (btn_exit)     btn_exit->OnClicked.AddDynamic(this, &UCmdDlg::OnExitClicked);

    if (btn_orders)   btn_orders->OnClicked.AddDynamic(this, &UCmdDlg::OnModeOrdersClicked);
    if (btn_theater)  btn_theater->OnClicked.AddDynamic(this, &UCmdDlg::OnModeTheaterClicked);
    if (btn_forces)   btn_forces->OnClicked.AddDynamic(this, &UCmdDlg::OnModeForcesClicked);
    if (btn_intel)    btn_intel->OnClicked.AddDynamic(this, &UCmdDlg::OnModeIntelClicked);
    if (btn_missions) btn_missions->OnClicked.AddDynamic(this, &UCmdDlg::OnModeMissionsClicked);

    // Cache pointers (safe even if null):
    Stars = Starshatter::GetInstance();
    CampaignPtr = Campaign::GetCampaign();

    // Apply any embedded FORM defaults if you provide GetLegacyFormText():
    const FString Frm = GetLegacyFormText();
    if (!Frm.IsEmpty())
    {
        FString Err;
        FParsedForm Parsed;
        if (ParseLegacyForm(Frm, Parsed, Err))
        {
            ParsedForm = Parsed;
            ApplyLegacyFormDefaults(ParsedForm);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("CmdDlg: ParseLegacyForm failed: %s"), *Err);
        }
    }
}

void UCmdDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Classic CmdDlg::ExecFrame updates every frame.
    ExecFrame();
}

// --------------------------------------------------------------------
// UBaseScreen overrides
// --------------------------------------------------------------------

void UCmdDlg::BindFormWidgets()
{
    // Labels:
    BindLabel(200, txt_group);
    BindLabel(201, txt_score);
    BindLabel(300, txt_name);
    BindLabel(301, txt_time);

    // Buttons:
    BindButton(100, btn_orders);
    BindButton(101, btn_theater);
    BindButton(102, btn_forces);
    BindButton(103, btn_intel);
    BindButton(104, btn_missions);

    BindButton(1, btn_save);
    BindButton(2, btn_exit);
}

FString UCmdDlg::GetLegacyFormText() const
{
    // You included three "form:" blocks (640/800/1024) in the same file.
    // Your current BaseScreen parser expects ONE form per text blob.
    //
    // Practical approach:
    // - Return the single form block that matches your current target resolution, OR
    // - Keep this empty and rely on UMG layout instead.
    //
    // For now: return empty to avoid parse failure.
    return FString();
}

// --------------------------------------------------------------------
// Public API
// --------------------------------------------------------------------

void UCmdDlg::SetManager(UCmpnScreen* InManager)
{
    CmpnScreen = InManager;
}

void UCmdDlg::ShowCmdDlg()
{
    CampaignPtr = Campaign::GetCampaign();

    if (txt_name)
    {
        if (CampaignPtr)
            txt_name->SetText(FText::FromString(CampaignPtr->Name()));
        else
            txt_name->SetText(FText::FromString(TEXT("No Campaign Selected")));
    }

    ShowMode();

    if (CampaignPtr)
    {
        const bool bTraining = CampaignPtr->IsTraining();

        if (btn_save)     btn_save->SetIsEnabled(!bTraining);
        if (btn_forces)   btn_forces->SetIsEnabled(!bTraining);
        if (btn_intel)    btn_intel->SetIsEnabled(!bTraining);
    }

    SetVisibility(ESlateVisibility::Visible);
}

void UCmdDlg::ExecFrame()
{
    if (!CampaignPtr)
        CampaignPtr = Campaign::GetCampaign();

    if (!CampaignPtr)
        return;

    // Player group:
    if (txt_group)
    {
        CombatGroup* G = CampaignPtr->GetPlayerGroup();
        if (G)
            txt_group->SetText(FText::FromString(G->GetDescription()));
    }

    // Score:
    if (txt_score)
    {
        const int32 TeamScore = CampaignPtr->GetPlayerTeamScore();
        const FString ScoreStr = FString::Printf(TEXT("Team Score: %d"), TeamScore);
        txt_score->SetText(FText::FromString(ScoreStr));

        // Classic: txt_score->SetTextAlign(DT_RIGHT);
        // In UMG, set justification on the TextBlock in the widget designer,
        // or do it once here if you want:
        txt_score->SetJustification(ETextJustify::Right);
    }

    // Time:
    if (txt_time)
    {
        const double T = CampaignPtr->GetTime();

        char DayTime[32] = { 0 };
        FormatDayTime(DayTime, T);

        txt_time->SetText(FText::FromString(UTF8_TO_TCHAR(DayTime)));
    }

    // Intel unread count -> change button label:
    const int32 Unread = CampaignPtr->CountNewEvents();

    if (txt_btn_intel)
    {
        if (Unread > 0)
            txt_btn_intel->SetText(FText::FromString(FString::Printf(TEXT("INTEL (%d)"), Unread)));
        else
            txt_btn_intel->SetText(FText::FromString(TEXT("INTEL")));
    }
    // If you do not have a separate text widget for the intel button, you can ignore this.
    // In UE, UButton itself has no SetText; you must set a nested UTextBlock.
}

void UCmdDlg::SetMode(ECOMMAND_MODE InMode)
{
    RouteMode(InMode);
}

// --------------------------------------------------------------------
// Internal behavior
// --------------------------------------------------------------------

void UCmdDlg::ShowMode()
{
    // Classic code uses SetButtonState(0/1).
    // In Unreal, you typically:
    // - swap styles (pressed/normal), or
    // - maintain an "active" visual via bindings.
    //
    // Here we keep it minimal: just ensure Mode is clamped.
    const uint8 M = (uint8)Mode;
    if (M > (uint8)ECOMMAND_MODE::MODE_MISSIONS)
        Mode = ECOMMAND_MODE::MODE_ORDERS;
}

void UCmdDlg::RouteMode(ECOMMAND_MODE NewMode)
{
    Mode = NewMode;
    ShowMode();

    if (!CmpnScreen)
    {
        UE_LOG(LogTemp, Warning, TEXT("CmdDlg: CmpnScreen is null (RouteMode)."));
        return;
    }

    switch (Mode)
    {
        case ECOMMAND_MODE::MODE_ORDERS: 
            CmpnScreen->ShowCmdOrdersDlg();  
            break;
        case ECOMMAND_MODE::MODE_THEATER: 
            CmpnScreen->ShowCmdTheaterDlg();
            break;
        case ECOMMAND_MODE::MODE_FORCES:   
            CmpnScreen->ShowCmdForceDlg();
            break;
        case ECOMMAND_MODE::MODE_INTEL:  
            CmpnScreen->ShowCmdIntelDlg();
            break;
        case ECOMMAND_MODE::MODE_MISSIONS:
            CmpnScreen->ShowCmdMissionsDlg();
            break;
        default:        
            CmpnScreen->ShowCmdOrdersDlg();
            break;
    }
}

// --------------------------------------------------------------------
// Button handlers
// --------------------------------------------------------------------

void UCmdDlg::OnModeOrdersClicked() { RouteMode(ECOMMAND_MODE::MODE_ORDERS); }
void UCmdDlg::OnModeTheaterClicked() { RouteMode(ECOMMAND_MODE::MODE_THEATER); }
void UCmdDlg::OnModeForcesClicked() { RouteMode(ECOMMAND_MODE::MODE_FORCES); }
void UCmdDlg::OnModeIntelClicked() { RouteMode(ECOMMAND_MODE::MODE_INTEL); }
void UCmdDlg::OnModeMissionsClicked() { RouteMode(ECOMMAND_MODE::MODE_MISSIONS); }

void UCmdDlg::OnSaveClicked()
{
    if (!CampaignPtr)
        CampaignPtr = Campaign::GetCampaign();

    if (CampaignPtr && CmpnScreen)
    {
        // Classic:
        // CmpFileDlg* fdlg = cmpn_screen->GetCmpFileDlg();
        // cmpn_screen->ShowCmpFileDlg();
        //
        // Unreal port depends on your UCmpnScreen API:
        CmpnScreen->ShowCmpFileDlg();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("CmdDlg: CampaignPtr or CmpnScreen is null (OnSaveClicked)."));
    }
}

void UCmdDlg::OnExitClicked()
{
    if (Stars)
    {
        Mouse::Show(false);
        Stars->SetGameMode(EMODE::MENU_MODE);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("CmdDlg: Starshatter instance is null (OnExitClicked)."));
    }
}
