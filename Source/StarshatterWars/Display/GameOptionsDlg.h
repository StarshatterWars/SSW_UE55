/*=============================================================================
    Project:        Starshatter Wars
    Studio:         Fractal Dev Studios
    Copyright:      (c) 2025-2026.
    All Rights Reserved.

    SUBSYSTEM:      StarshatterWars (Unreal Engine)
    FILE:           GameOptionsDlg.h
    AUTHOR:         Carlos Bott

    OVERVIEW
    ========
    UGameOptionsDlg
    - Gameplay options subpage hosted by UOptionsScreen.
    - Replaces legacy "Options" page from Starshatter OptionsScreen.
    - Reads/writes PlayerCharacter + Ship + HUDView (legacy globals).
    - Apply/Cancel is orchestrated by UOptionsScreen (single source of truth).

    NOTES
    =====
    - Uses AddUniqueDynamic to avoid duplicate delegate ensures.
    - Tab buttons route back to UOptionsScreen to switch pages.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "GameOptionsDlg.generated.h"

class UButton;
class UComboBoxString;
class UTextBlock;

class UOptionsScreen;

UCLASS()
class STARSHATTERWARS_API UGameOptionsDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UGameOptionsDlg(const FObjectInitializer& ObjectInitializer);

    void SetOptionsManager(UOptionsScreen* InManager) { OptionsManager = InManager; }
    UOptionsScreen* GetOptionsManager() const { return OptionsManager; }

    void Show();
    virtual void ExecFrame(double DeltaTime) override;

    void Apply();
    void Cancel();

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;

    virtual void BindFormWidgets() override;
    virtual FString GetLegacyFormText() const override;

    virtual void HandleAccept() override;
    virtual void HandleCancel() override;

private:
    void BindDelegates();

    void RefreshFromModel();
    void PushToModel();

private:

    bool bClosed = true;
    bool bDelegatesBound = false;

    // Snapshot (kept simple; we re-pull on RefreshFromModel)
    int32 FlightModel = 0;
    int32 LandingModel = 0;
    int32 FlyingStart = 0;
    int32 AIDifficultyIndex = 0;
    int32 HudModeIndex = 0;
    int32 HudColorIndex = 0;
    int32 FriendlyFireIndex = 0;
    int32 GridModeIndex = 0;
    int32 GunsightIndex = 0;

protected:
    // ------------------------------------------------------------
    // UMG bindings (optional)
    // ------------------------------------------------------------

    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> description;

    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UComboBoxString> flight_model;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UComboBoxString> flying_start;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UComboBoxString> landings;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UComboBoxString> ai_difficulty;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UComboBoxString> hud_mode;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UComboBoxString> hud_color;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UComboBoxString> ff_mode;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UComboBoxString> grid_mode;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UComboBoxString> gunsight;

    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> ApplyBtn;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> CancelBtn;

    // Tabs
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> vid_btn;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> aud_btn;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> ctl_btn;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> opt_btn; // GAME tab (this page)
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> mod_btn;

private:
    // Apply/Cancel
    UFUNCTION() void OnApplyClicked();
    UFUNCTION() void OnCancelClicked();

    // Tabs
    UFUNCTION() void OnAudioClicked();
    UFUNCTION() void OnVideoClicked();
    UFUNCTION() void OnControlsClicked();
    UFUNCTION() void OnGameClicked();
    UFUNCTION() void OnModClicked();

    // Combos
    UFUNCTION() void OnFlightModelChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnFlyingStartChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnLandingsChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnAIDifficultyChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnHudModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnHudColorChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnFfModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnGridModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnGunsightChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
};
