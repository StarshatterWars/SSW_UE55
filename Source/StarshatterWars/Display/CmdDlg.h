/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         CmdDlg.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UCmdDlg
    - Unreal port of Starshatter CmdDlg helper/controller.
    - In classic code, CmdDlg is NOT a FormWindow; it binds to an existing FormWindow owned by CmpnScreen.
      In Unreal, we treat this as a UBaseScreen-derived widget that owns its own UMG widgets and uses FORM IDs.
    - Responsibilities:
        * Shows current campaign title, group description, team score, and time.
        * Provides mode buttons (Orders/Theater/Forces/Intel/Missions) and Save/Exit.
        * Updates Intel button label with unread count.
        * Enables/disables Save/Forces/Intel when campaign is training.
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "Campaign.h"
#include "Starshatter.h"
#include "CmpnScreen.h"
#include "GameStructs.h"
#include "CmdDlg.generated.h"

class UTextBlock;
class UButton;
class Starshatter;
class Campaign;
class CombatGroup;
class UCmpFileDlg;      // Your file dialog widget (port of CmpFileDlg)

/**
 * Operational Command Dialog
 */
UCLASS()
class STARSHATTERWARS_API UCmdDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UCmdDlg(const FObjectInitializer& ObjectInitializer);

    // ============================================================
    // UUserWidget lifecycle
    // ============================================================
protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // ============================================================
    // UBaseScreen overrides
    // ============================================================
public:
    virtual void BindFormWidgets() override;
    virtual FString GetLegacyFormText() const override;

    // ============================================================
    // Public API (ported from CmdDlg)
    // ============================================================
public:
    void SetManager(UCmpnScreen* InManager);
    void ShowCmdDlg();
    void ExecFrame();
    void SetMode(ECOMMAND_MODE InMode);

protected:
    /** Equivalent to ShowMode() */
    void ShowMode();

    // ============================================================
    // Button handlers
    // ============================================================
protected:
    UFUNCTION()
    void OnSaveClicked();

    UFUNCTION()
    void OnExitClicked();

    UFUNCTION()
    void OnModeOrdersClicked();

    UFUNCTION()
    void OnModeTheaterClicked();

    UFUNCTION()
    void OnModeForcesClicked();

    UFUNCTION()
    void OnModeIntelClicked();

    UFUNCTION()
    void OnModeMissionsClicked();

private:
    void RouteMode(ECOMMAND_MODE NewMode);

private:
    // ============================================================
    // Manager / dependencies
    // ============================================================
    UCmpnScreen* CmpnScreen = nullptr;
    
    Campaign* CampaignPtr = nullptr;
    Starshatter* Stars = nullptr;

    // ============================================================
    // FORM bound widgets (IDs match .frm)
    // ============================================================

    // Labels:
    // 200 = group description
    UPROPERTY(meta = (BindWidgetOptional), Transient)
    UTextBlock* txt_group = nullptr;

    // 201 = score
    UPROPERTY(meta = (BindWidgetOptional), Transient)
    UTextBlock* txt_score = nullptr;

    // 300 = campaign name/title
    UPROPERTY(meta = (BindWidgetOptional), Transient)
    UTextBlock* txt_name = nullptr;

    // 301 = daytime string
    UPROPERTY(meta = (BindWidgetOptional), Transient)
    UTextBlock* txt_time = nullptr;

    // Buttons:
    // 100..104 = mode buttons
    UPROPERTY(meta = (BindWidgetOptional), Transient) UButton* btn_orders = nullptr; // 100
    UPROPERTY(meta = (BindWidgetOptional), Transient) UButton* btn_theater = nullptr; // 101
    UPROPERTY(meta = (BindWidgetOptional), Transient) UButton* btn_forces = nullptr; // 102
    UPROPERTY(meta = (BindWidgetOptional), Transient) UButton* btn_intel = nullptr; // 103
    UPROPERTY(meta = (BindWidgetOptional), Transient) UButton* btn_missions = nullptr; // 104

    // 1 = save, 2 = exit
    UPROPERTY(meta = (BindWidgetOptional), Transient) UButton* btn_save = nullptr; // 1
    UPROPERTY(meta = (BindWidgetOptional), Transient) UButton* btn_exit = nullptr; // 2

    // Optional text blocks inside buttons (if your UMG splits text)
    UPROPERTY(meta = (BindWidgetOptional), Transient) UTextBlock* txt_btn_intel = nullptr;

    // ============================================================
    // State
    // ============================================================
    ECOMMAND_MODE Mode = ECOMMAND_MODE::MODE_ORDERS;
};
