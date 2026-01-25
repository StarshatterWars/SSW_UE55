/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         CmdMissionsDlg.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UCmdMissionsDlg (Unreal port of CmdMissionsDlg)
*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CmdDlg.h"
#include "GameStructs.h"
#include "CmdMissionsDlg.generated.h"

// Forward declarations (UMG)
class UButton;
class UTextBlock;
class UListView;
class URichTextBlock;

// Starshatter core forward declarations
class Starshatter;
class Campaign;
class Mission;
class MissionInfo;
class UCmpnScreen;

UCLASS()
class STARSHATTERWARS_API UCmdMissionListItem : public UObject
{
    GENERATED_BODY()

public:
    // Columns
    UPROPERTY() FString MissionName;
    UPROPERTY() FString MissionType;
    UPROPERTY() FString StartTime;

    // Mission id (legacy list item data stored info->id)
    UPROPERTY() int32 MissionId = 0;

    // Backing pointer (optional; legacy MissionInfo* may not be stable across rebuilds)
    Mission* MissionPtr = nullptr;
};

UCLASS()
class STARSHATTERWARS_API UCmdMissionsDlg : public UUserWidget
{
    GENERATED_BODY()

public:
    UCmdMissionsDlg(const FObjectInitializer& ObjectInitializer);

    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
    void SetManager(UCmpnScreen* InManager);
    void ShowMissionsDlg();

private:
    void BindFormWidgets();

    // Core tick logic (mirrors legacy CmdMissionsDlg::ExecFrame)
    void ExecFrame();

    // Header refresh (mirrors CmdDlg::ExecFrame header portion)
    void ExecHeaderFrame();

    // Mission list maintenance
    void RebuildMissionList();
    void AppendNewMissionsIfAny();
    void ValidateSelectionStillExists();

    // Selection helpers
    int32 GetSelectedMissionId() const;
    void SetSelectedMissionId(int32 MissionId);

    // Description panel
    void ClearDescription();
    void SetDescriptionForMissionInfo(MissionInfo* Info);

    // Eligibility
    bool CanAcceptMission(MissionInfo* Info) const;

    // Update accept button enable state
    void UpdateAcceptEnabled();

    // Routed mode change
    void SetModeAndRoute(ECOMMAND_MODE InMode);

private:
    // UMG bindings (BindWidgetOptional: safe if names differ in your widget BP)
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* txt_group = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* txt_score = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* txt_name = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* txt_time = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UButton* btn_orders = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* btn_theater = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* btn_forces = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* btn_intel = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* btn_missions = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UButton* btn_save = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* btn_exit = nullptr;

    // Missions tab widgets
    UPROPERTY(meta = (BindWidgetOptional)) UListView* lst_missions = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) URichTextBlock* txt_desc = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* btn_accept = nullptr;

private:
    // Manager/systems
    UCmpnScreen* Manager = nullptr;

    Starshatter* Stars = nullptr;
    Campaign* CampaignPtr = nullptr;

    Mission* SelectedMission = nullptr;

    UCmdDlg::ECmdMode Mode = UCmdDlg::ECmdMode::MODE_MISSIONS;

private:
    // UFUNCTION handlers (must be UFUNCTION for AddDynamic)
    UFUNCTION() void OnSaveClicked();
    UFUNCTION() void OnExitClicked();

    UFUNCTION() void OnModeOrdersClicked();
    UFUNCTION() void OnModeTheaterClicked();
    UFUNCTION() void OnModeForcesClicked();
    UFUNCTION() void OnModeIntelClicked();
    UFUNCTION() void OnModeMissionsClicked();

    UFUNCTION() void OnAcceptClicked();

    // ListView delegates
    UFUNCTION() void OnMissionItemClicked(UObject* Item);
};
