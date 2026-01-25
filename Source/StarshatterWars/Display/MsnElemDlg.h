/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         MsnElemDlg.h
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    Mission Editor Element Dialog (UMG UserWidget) — Unreal port of the legacy
    active window / form-driven mission element editor screen.
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"

// Minimal Unreal includes requested for headers:
#include "Math/Vector.h"              // FVector
#include "Math/Color.h"               // FColor
#include "Math/UnrealMathUtility.h"   // Math

#include "MsnElemDlg.generated.h"

// Forward declarations (UMG widgets):
class UButton;
class UComboBoxString;
class UEditableTextBox;
class UTextBlock;

// Legacy / Starshatter core forward declarations (non-UObject classes):
class MenuScreen;
class UMsnEditDlg;
class Mission;
class MissionElement;

/**
 * Mission Element Editor screen (UMG).
 * Port of legacy FormWindow-based MsnElemDlg into a UBaseScreen-derived widget.
 *
 * Notes:
 * - Legacy FORM control IDs should be bound in BindFormWidgets().
 * - MissionElement location is treated as FVector in the Unreal port.
 */
UCLASS()
class STARSHATTERWARS_API UMsnElemDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UMsnElemDlg(const FObjectInitializer& ObjectInitializer);

public:
    // ---- Legacy-style API --------------------------------------------

    void Show();
    void ExecFrame();
    void SetMission(Mission* inMission);
    void SetMissionElement(MissionElement* inElem);

public:
    // ---- UBaseScreen --------------------------------------------------

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

    UFUNCTION() void HandleClassChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void HandleDesignChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void HandleObjectiveChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION() void HandleIFFTextChanged(const FText& NewText);

protected:
    // ---- Internal operations -----------------------------------------

    void UpdateTeamInfo();
    bool CanCommand(const MissionElement* commander, const MissionElement* subordinate) const;

protected:
    // ---- External/legacy manager -------------------------------------

    // NOTE: MenuScreen is a legacy, non-UObject class. Intentionally not a UPROPERTY.
    MenuScreen* manager = nullptr;

protected:
    // ---- Widgets ------------------------------------------------------

    UPROPERTY(meta = (BindWidgetOptional)) UEditableTextBox* EdtName = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* CmbClass = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* CmbDesign = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* CmbSkin = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UEditableTextBox* EdtSize = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UEditableTextBox* EdtIFF = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* CmbRole = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* CmbRegion = nullptr;

    // Location (legacy edt_loc_x/y/z):
    UPROPERTY(meta = (BindWidgetOptional)) UEditableTextBox* EdtLocX = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UEditableTextBox* EdtLocY = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UEditableTextBox* EdtLocZ = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* CmbHeading = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UEditableTextBox* EdtHoldTime = nullptr;

    // Flags / toggles:
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnPlayer = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnPlayable = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnAlert = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnCommandAI = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UEditableTextBox* EdtRespawns = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* CmbCarrier = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* CmbSquadron = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* CmbCommander = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* CmbIntel = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* CmbLoadout = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* CmbObjective = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* CmbTarget = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnAccept = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnCancel = nullptr;

protected:
    // ---- Data ---------------------------------------------------------

    Mission* mission = nullptr; // legacy pointer
    MissionElement* elem = nullptr; // legacy pointer
    bool            exit_latch = true;
};
