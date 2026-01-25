/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         MsnEditDlg.h
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    Mission Editor Dialog (UMG UserWidget) — Unreal port of the legacy
    active window / form-driven mission editor screen.
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"

#include "MsnEditDlg.generated.h"

// Forward declarations (keep header light):
class UTextBlock;
class UButton;
class UEditableTextBox;
class UComboBoxString;
class UListView;

// Starshatter core forward declarations (non-UObject legacy types are permitted):
class MenuScreen;
class Campaign;
class Mission;
class MissionInfo;

/**
 * Mission Editor Dialog screen (UMG).
 * Port of the legacy FormWindow-based MsnEditDlg into a UBaseScreen-derived widget.
 *
 * Notes:
 * - Legacy FORM control IDs should be bound in BindFormWidgets().
 * - Screen logic should be implemented in NativeTick / handlers, or delegated to manager.
 */
UCLASS()
class STARSHATTERWARS_API UMsnEditDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UMsnEditDlg(const FObjectInitializer& ObjectInitializer);

    // ---- Legacy-style API --------------------------------------------

    /** Equivalent to legacy Show() */
    UFUNCTION(BlueprintCallable, Category = "Starshatter Wars|Mission Editor")
    void Show();

    /** Equivalent to legacy ExecFrame() */
    UFUNCTION(BlueprintCallable, Category = "Starshatter Wars|Mission Editor")
    void ExecFrame();

    /** Equivalent to legacy ScrapeForm() */
    UFUNCTION(BlueprintCallable, Category = "Starshatter Wars|Mission Editor")
    void ScrapeForm();

    Mission* GetMission() const { return mission; }
    void     SetMission(Mission* m);
    void     SetMissionInfo(MissionInfo* m);

public:
    // ---- UBaseScreen --------------------------------------------------

    /** Bind legacy FORM IDs to widgets here */
    virtual void BindFormWidgets() override;

protected:
    // ---- UUserWidget lifecycle ---------------------------------------

    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
    // ---- UI Event Handlers -------------------------------------------

    UFUNCTION() void HandleAcceptClicked();
    UFUNCTION() void HandleCancelClicked();

    UFUNCTION() void HandleElemAddClicked();
    UFUNCTION() void HandleElemEditClicked();
    UFUNCTION() void HandleElemDelClicked();
    UFUNCTION() void HandleElemIncClicked();
    UFUNCTION() void HandleElemDecClicked();

    UFUNCTION() void HandleEventAddClicked();
    UFUNCTION() void HandleEventEditClicked();
    UFUNCTION() void HandleEventDelClicked();
    UFUNCTION() void HandleEventIncClicked();
    UFUNCTION() void HandleEventDecClicked();

    UFUNCTION() void HandleTabSitClicked();
    UFUNCTION() void HandleTabPkgClicked();
    UFUNCTION() void HandleTabMapClicked();

protected:
    // ---- Internal operations -----------------------------------------

    void DrawPackages();
    void ShowTab(int tab);

protected:
    // ---- External/legacy manager -------------------------------------

    // NOTE: MenuScreen is a legacy, non-UObject class. Intentionally not a UPROPERTY.
    MenuScreen* manager = nullptr;

protected:
    // ---- Widgets ------------------------------------------------------

    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnAccept = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnCancel = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnElemAdd = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnElemEdit = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnElemDel = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnElemInc = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnElemDec = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnEventAdd = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnEventEdit = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnEventDel = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnEventInc = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnEventDec = nullptr;

    // Tabs:
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnSit = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnPkg = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnMap = nullptr;

    // Form fields:
    UPROPERTY(meta = (BindWidgetOptional)) UEditableTextBox* TxtName = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* CmbType = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* CmbSystem = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* CmbRegion = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UEditableTextBox* TxtDescription = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UEditableTextBox* TxtSituation = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UEditableTextBox* TxtObjective = nullptr;

    // Lists:
    UPROPERTY(meta = (BindWidgetOptional)) UListView* LstElem = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UListView* LstEvent = nullptr;

protected:
    // ---- Data ---------------------------------------------------------

    Mission* mission = nullptr;   // legacy pointer
    MissionInfo* mission_info = nullptr;   // legacy pointer

    int  current_tab = 0;
    bool exit_latch = false;
};
