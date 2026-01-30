/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         MissionDebriefDlg.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Mission Debriefing dialog (Unreal)
    - FORM-driven dialog (DebriefDlg.frm) port.
    - Shows mission header info, objectives/situation, score,
      active units list, per-unit summary stats, and mission log.
    - Owned/managed by MissionPlanner (tab flow / show-hide).
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "MissionDebriefDlg.generated.h"

// Forward declarations (your legacy/ported sim classes):
class Campaign;
class Mission;
class MissionInfo;
class Sim;
class Ship;
class StarSystem;
class MissionElement;
class Player;
class ShipStats;
class SimEvent;

// Forward declarations (your FORM control wrappers):
class UActiveWindow;
class UButton;
class UListBox;

class UMissionPlanner;

UCLASS()
class STARSHATTERWARS_API UMissionDebriefDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UMissionDebriefDlg(const FObjectInitializer& ObjectInitializer);

    // ----------------------------------------------------------------
    // Legacy-like lifecycle
    // ----------------------------------------------------------------
    virtual void RegisterControls();
    virtual void ExecFrame(float DeltaSeconds);
    virtual void Show();
    virtual void Hide();

    // ----------------------------------------------------------------
    // Operations (event handlers)
    // ----------------------------------------------------------------
    void OnClose();
    void OnUnit();

protected:
    void DrawUnits();

protected:
    // UUserWidget lifecycle
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
    // MissionPlanner will set this after CreateWidget (raw pointer by request):
    void SetManager(UMissionPlanner* InManager) { Manager = InManager; }
    UMissionPlanner* GetManager() const { return Manager; }

private:
    // ----------------------------------------------------------------
    // Owner/Manager (raw pointer by request)
    // ----------------------------------------------------------------
    UMissionPlanner* Manager = nullptr;

    // ----------------------------------------------------------------
    // FORM controls (raw pointers by request)
    // ----------------------------------------------------------------
    UButton* CloseButton = nullptr;

    UActiveWindow* MissionName = nullptr;
    UActiveWindow* MissionSystem = nullptr;
    UActiveWindow* MissionSector = nullptr;
    UActiveWindow* MissionTimeStart = nullptr;

    UActiveWindow* Objectives = nullptr;
    UActiveWindow* Situation = nullptr;
    UActiveWindow* MissionScore = nullptr;

    UListBox* UnitList = nullptr;
    UListBox* SummaryList = nullptr;
    UListBox* EventList = nullptr;

    // ----------------------------------------------------------------
    // Legacy state (raw pointers, non-UObject)
    // ----------------------------------------------------------------
    Campaign* CampaignPtr = nullptr;
    Mission* MissionPtr = nullptr;
    MissionInfo* InfoPtr = nullptr;
    int32        UnitIndex = 0;

    Sim* SimPtr = nullptr;
    Ship* PlayerShip = nullptr;

    bool         bIsShown = false;
};
