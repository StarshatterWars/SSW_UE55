/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    ORIGINAL AUTHOR AND STUDIO:
        John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         MissionSelectDlg.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    MissionSelectDlg
    - Mission selection dialog (Single / Custom / Multiplayer).
    - Lists campaigns and missions.
    - Displays mission description.
    - Launches mission on Accept.
*/

#pragma once

#include "BaseScreen.h"

// Minimal Unreal
#include "Math/Vector.h"

// Starshatter core
#include "Text.h"
#include "List.h"

#include "MissionSelectDlg.generated.h"

// Forward declarations (keep header light)
class Campaign;
class Mission;
class Starshatter;
class UButton;
class UListView;
class UComboBoxString;
class URichTextBlock;

UCLASS()
class STARSHATTERWARS_API UMissionSelectDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UMissionSelectDlg(const FObjectInitializer& ObjectInitializer);

    // Lifecycle
    virtual void NativeConstruct() override;
    virtual void Show() override;
    virtual void ExecFrame(double DeltaTime) override;

protected:
    // Event wiring
    void WireEvents();

    // Handlers (legacy mappings)
    UFUNCTION() void OnAcceptClicked();
    UFUNCTION() void OnCancelClicked();
    UFUNCTION() void OnNewClicked();
    UFUNCTION() void OnEditClicked();
    UFUNCTION() void OnDeleteClicked();
    UFUNCTION() void OnDeleteConfirm();

    UFUNCTION() void OnCampaignSelected();
    UFUNCTION() void OnMissionSelected();

protected:
    // Starshatter state
    Starshatter* Stars = nullptr;
    Campaign* CampaignPtr = nullptr;

    int32 SelectedMission = -1;
    int32 MissionId = 0;
    bool  bEditable = false;
};
