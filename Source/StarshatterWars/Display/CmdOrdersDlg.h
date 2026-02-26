/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         CmdOrdersDlg.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UCmdOrdersDlg (Unreal port of CmdOrdersDlg)
*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CmdOrdersDlg.generated.h"

// UMG forward declarations
class UButton;
class URichTextBlock;
class UTextBlock;

// Your campaign screen manager
class UCmpnScreen;

// Starshatter core forward declarations
class Starshatter;
class Campaign;

UCLASS()
class STARSHATTERWARS_API UCmdOrdersDlg : public UUserWidget
{
    GENERATED_BODY()

public:
    UCmdOrdersDlg(const FObjectInitializer& ObjectInitializer);

    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
    void SetManager(UCmpnScreen* InManager);

    // Show this tab/dialog (mirrors legacy Show)
    void ShowOrdersDlg();

    // Called per-frame (mirrors legacy CmdDlg::ExecFrame + local updates)
    void ExecFrame();

protected:
    // Tab routing
    void SetModeAndHighlight(int32 InMode);

    UFUNCTION() void OnModeOrdersClicked();
    UFUNCTION() void OnModeTheaterClicked();
    UFUNCTION() void OnModeForcesClicked();
    UFUNCTION() void OnModeIntelClicked();
    UFUNCTION() void OnModeMissionsClicked();

    // Save/Exit
    UFUNCTION() void OnSaveClicked();
    UFUNCTION() void OnExitClicked();

protected:
    // Header widgets (match your existing BindFormWidgets approach or BindWidget names)
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* txt_group = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* txt_score = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* txt_name = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* txt_time = nullptr;

    // Tab buttons
    UPROPERTY(meta = (BindWidgetOptional)) UButton* btn_orders = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* btn_theater = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* btn_forces = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* btn_intel = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* btn_missions = nullptr;

    // Footer buttons
    UPROPERTY(meta = (BindWidgetOptional)) UButton* btn_save = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* btn_exit = nullptr;

    // Orders body (legacy control id 400 was a scrollable text control)
    UPROPERTY(meta = (BindWidgetOptional)) URichTextBlock* txt_orders_rich = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* txt_orders = nullptr;

protected:
    UCmpnScreen* Manager = nullptr;

    Starshatter* Stars = nullptr;
    Campaign* CampaignPtr = nullptr;

    int32 Mode = 0;
};
