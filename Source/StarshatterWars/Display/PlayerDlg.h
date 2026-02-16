/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.
    All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         PlayerDlg.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Player Logbook dialog (UE port)
    - Uses BaseScreen "Options-subpanel" pattern:
      AutoVBox + AddLabeledRow() for label-left / control-right rows.
    - Builds full legacy PlayerDlg.frm control set (UI side).
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "PlayerDlg.generated.h"

class UButton;
class UEditableTextBox;
class UImage;
class UListView;
class UTextBlock;
class UUniformGridPanel;

class UPlayerRosterItem;
class PlayerCharacter;
class UMenuScreen;

UCLASS()
class STARSHATTERWARS_API UPlayerDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UPlayerDlg(const FObjectInitializer& ObjectInitializer);

    void InitializeDlg(UMenuScreen* InManager);

    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

    void ShowDlg();
    void HideDlg();

protected:
    // ------------------------------------------------------------
    // UI build + wiring
    // ------------------------------------------------------------
    void RegisterControls();

    void BuildRoster();
    void RefreshRoster();

    void BuildFormRows();      // creates all rows/controls into BaseScreen AutoVBox
    void RebuildFromModel();   // model -> UI
    void CommitToModel();      // UI -> model

    PlayerCharacter* GetSelectedPlayer() const;

    // ------------------------------------------------------------
    // Events
    // ------------------------------------------------------------
    UFUNCTION()
    void OnRosterSelectionChanged(UObject* SelectedItem);

    UFUNCTION()
    void OnAdd();

    UFUNCTION()
    void OnDel();

    UFUNCTION()
    void OnApply();

    UFUNCTION()
    void OnCancel();

    // ------------------------------------------------------------
    // Helpers
    // ------------------------------------------------------------
    static FString FormatTimeHMS(double Seconds);
    static FString FormatDateFromUnixSeconds(int64 UnixSeconds);

protected:
    // ------------------------------------------------------------
    // Manager
    // ------------------------------------------------------------
    UPROPERTY()
    UMenuScreen* manager = nullptr;

    // ------------------------------------------------------------
    // LEFT: roster + Create/Delete (bound from UMG if present)
    // ------------------------------------------------------------
    UPROPERTY(meta = (BindWidgetOptional))
    UListView* lst_roster = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    UButton* btn_add = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    UButton* btn_del = nullptr;

    // Optional local Save/Cancel buttons (if your WBP has them).
    // Otherwise BaseScreen::ApplyButton / CancelButton are used.
    UPROPERTY(meta = (BindWidgetOptional))
    UButton* BtnSave = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    UButton* BtnCancel = nullptr;

    // ------------------------------------------------------------
    // RIGHT: controls built in code (legacy form content)
    // ------------------------------------------------------------

    // Edits:
    UPROPERTY()
    UEditableTextBox* edt_name = nullptr;

    UPROPERTY()
    UEditableTextBox* edt_password = nullptr;

    UPROPERTY()
    UEditableTextBox* edt_squadron = nullptr;

    UPROPERTY()
    UEditableTextBox* edt_signature = nullptr;

    // Read-only stats labels:
    UPROPERTY()
    UTextBlock* txt_created = nullptr;

    UPROPERTY()
    UTextBlock* txt_flighttime = nullptr;

    UPROPERTY()
    UTextBlock* txt_missions = nullptr;

    UPROPERTY()
    UTextBlock* txt_kills = nullptr;

    UPROPERTY()
    UTextBlock* txt_losses = nullptr;

    UPROPERTY()
    UTextBlock* txt_points = nullptr;

    UPROPERTY()
    UTextBlock* txt_rankname = nullptr;

    // Rank insignia:
    UPROPERTY()
    UImage* img_rank = nullptr;

    // Medals grid (3x5):
    UPROPERTY()
    UUniformGridPanel* medals_grid = nullptr;

    UPROPERTY()
    TArray<TObjectPtr<UImage>> MedalImages; // 15

    // Chat macros 1..9,0:
    UPROPERTY()
    TArray<TObjectPtr<UEditableTextBox>> MacroEdits; // 10

    // ------------------------------------------------------------
    // State
    // ------------------------------------------------------------
    UPROPERTY()
    TObjectPtr<UPlayerRosterItem> selected_item = nullptr;

    PlayerCharacter* SelectedPlayer = nullptr;
};
