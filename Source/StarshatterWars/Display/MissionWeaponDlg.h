/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         MissionWeaponDlg.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    MissionWeaponDlg (Unreal)
    - FORM-driven mission briefing "WEP" tab.
    - Ports legacy MsnWepDlg behavior:
      * Shows player element name/type/weight
      * Shows standard loadouts list and custom station grid
      * Clicking grid toggles mounts and updates MissionLoad
      * Selecting a standard loadout applies ShipLoad mapping
    - Raw pointers only (no UPROPERTY(Transient)).
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "MissionWeaponDlg.generated.h"

// Forward decls (legacy sim side)
class Campaign;
class Mission;
class MissionElement;
class ShipDesign;
class WeaponDesign;
class HardPoint;
class MissionLoad;
class ShipLoad;

// Forward decls (FORM/UI controls in your framework)
class UFormLabel;
class UFormButton;
class UFormListBox;
class UFormImageBox;

class UMissionPlanner; // the manager/controller (your definitive MissionPlanner)

UCLASS()
class STARSHATTERWARS_API UMissionWeaponDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UMissionWeaponDlg(const FObjectInitializer& ObjectInitializer);

    // ----------------------------------------------------------------
    // UBaseScreen / UUserWidget
    // ----------------------------------------------------------------
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // ----------------------------------------------------------------
    // Legacy-like API surface
    // ----------------------------------------------------------------
    virtual void Show();
    virtual void Hide();
    virtual bool IsShown() const;

    void SetManager(UMissionPlanner* InMgr) { Manager = InMgr; }

protected:
    // ----------------------------------------------------------------
    // FORM wiring
    // ----------------------------------------------------------------
    void RegisterControls();
    void RegisterEvents();

    // ----------------------------------------------------------------
    // Core behavior (ported from MsnWepDlg)
    // ----------------------------------------------------------------
    void FindPlayerElement();
    void SetupControls();
    void BuildLists();

    int32 LoadToPointIndex(int32 HardPointIndex) const;
    int32 PointIndexToLoad(int32 HardPointIndex, int32 DesignIndexInHardPoint) const;

    void UpdateGridPicturesForStation(int32 Station);
    void UpdateAllGridPictures();
    void UpdateWeightLabelFromLoads();
    void ClearLoadoutSelection();

    // ----------------------------------------------------------------
    // Event handlers
    // ----------------------------------------------------------------
    void OnCommit();
    void OnCancel();
    void OnTabButton(int32 TabId);

    void OnMount(UFormButton* ClickedButton);
    void OnLoadoutSelectionChanged(int32 SelectedIndex);

private:
    // ----------------------------------------------------------------
    // Manager + sim pointers
    // ----------------------------------------------------------------
    UMissionPlanner* Manager = nullptr;

    Campaign* CampaignPtr = nullptr;
    Mission* MissionPtr = nullptr;

    MissionElement* PlayerElem = nullptr;

    // ----------------------------------------------------------------
    // FORM controls (raw pointers; owned by widget tree / form runtime)
    // ----------------------------------------------------------------
    UFormLabel* LblElement = nullptr; // 601
    UFormLabel* LblType = nullptr; // 602
    UFormLabel* LblWeight = nullptr; // 603
    UFormListBox* LoadoutList = nullptr; // 604

    UFormImageBox* Beauty = nullptr; // 300 (optional)
    UFormLabel* PlayerDesc = nullptr; // 301 (optional)

    UFormLabel* LblStation[8] = { nullptr }; // 401..408
    UFormLabel* LblDesc[8] = { nullptr }; // 500,510..570 (label rows)
    UFormButton* BtnLoad[8][8] = { { nullptr } }; // 501..578 (grid)

    // ----------------------------------------------------------------
    // Data model (ported arrays)
    // ----------------------------------------------------------------
    WeaponDesign* Designs[8] = { nullptr };   // unique weapon designs
    bool          Mounts[8][8] = { { false } }; // [designIdx][stationIdx]
    int32         Loads[8] = { -1,-1,-1,-1,-1,-1,-1,-1 }; // station->designIdx or -1
    int32         FirstStation = 0; // centering offset like legacy

    // ----------------------------------------------------------------
    // Visual assets (LED off/on)
    // Your FORM system likely maps picture tokens; keep as names.
    // ----------------------------------------------------------------
    FName LedOffName = TEXT("LED0"); // from "LED0.pcx"
    FName LedOnName = TEXT("LED1"); // from "LED1.pcx"

    // ----------------------------------------------------------------
    // State
    // ----------------------------------------------------------------
    bool bShown = false;

    // Key debouncing for Enter-to-commit (legacy VK_RETURN)
    bool bEnterWasDown = false;
};
