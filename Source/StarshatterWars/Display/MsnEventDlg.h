/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         MsnEventDlg.h
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    Mission Editor Event Dialog (UMG UserWidget) — Unreal port of the legacy
    mission event editor screen.
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"

// Minimal Unreal includes requested for headers:
#include "Math/Vector.h"              // FVector
#include "Math/Color.h"               // FColor
#include "Math/UnrealMathUtility.h"   // Math

#include "MsnEventDlg.generated.h"

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
class MissionEvent;

/**
 * Mission Event Editor screen (UMG).
 * Port of legacy FormWindow-based MsnEventDlg into a UBaseScreen-derived widget.
 */
UCLASS()
class STARSHATTERWARS_API UMsnEventDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UMsnEventDlg(const FObjectInitializer& ObjectInitializer);

public:
    // ---- Legacy-style API --------------------------------------------

    void Show();
    void ExecFrame();
    void SetMission(Mission* inMission);
    void SetMissionEvent(MissionEvent* inEvent);

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

    UFUNCTION() void HandleEventChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void HandleEventShipChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void HandleTriggerChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

protected:
    // ---- Internal operations -----------------------------------------

    void FillShipList(UComboBoxString* Combo, const char* SelectedName);
    void FillRgnList(UComboBoxString* Combo, const char* SelectedName);

protected:
    // ---- External/legacy manager -------------------------------------

    // NOTE: MenuScreen is a legacy, non-UObject class. Intentionally not a UPROPERTY.
    MenuScreen* manager = nullptr;

protected:
    // ---- Widgets ------------------------------------------------------

    // Legacy "lbl_id" was an ActiveWindow label. In UMG, use a TextBlock:
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* LblId = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UEditableTextBox* EdtTime = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UEditableTextBox* EdtDelay = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* CmbEvent = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* CmbEventShip = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* CmbEventSource = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* CmbEventTarget = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UEditableTextBox* EdtEventParam = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UEditableTextBox* EdtEventChance = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UEditableTextBox* EdtEventSound = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UEditableTextBox* EdtEventMessage = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* CmbTrigger = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* CmbTriggerShip = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* CmbTriggerTarget = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UEditableTextBox* EdtTriggerParam = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnAccept = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* BtnCancel = nullptr;

protected:
    // ---- Data ---------------------------------------------------------

    Mission* mission = nullptr; // legacy pointer
    MissionEvent* event = nullptr; // legacy pointer
};
