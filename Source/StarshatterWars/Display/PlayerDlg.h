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
    Player dialog (UE port)
    - BaseScreen-style dialog with Apply/Cancel
    - Displays and edits PlayerCharacter roster entries
    - Uses UListView with UObject list items (UPlayerRosterItem)
    - List row visuals handled by UPlayerRosterEntry
*/

#pragma once

#include "CoreMinimal.h"

#include "BaseScreen.h"

class UButton;
class UEditableTextBox;
class UImage;
class UListView;
class UTextBlock;

class UPlayerRosterItem;

class PlayerCharacter;
class UMenuScreen;

#include "PlayerDlg.generated.h"

// +--------------------------------------------------------------------+

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
    void RegisterControls();

    void BuildRoster();
    void RefreshRoster();

    void UpdatePlayer();   // commit UI -> player
    void ShowPlayer();     // player -> UI

    PlayerCharacter* GetSelectedPlayer() const;

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

    // Helpers:
    static FString FormatTimeHMS(double Seconds);
    static FString FormatDateFromUnixSeconds(int64 UnixSeconds);

protected:
    // -----------------------------------------------------------------
    // Manager
    // -----------------------------------------------------------------

    UPROPERTY()
    UMenuScreen* manager = nullptr;

    // -----------------------------------------------------------------
    // Widgets (BindWidgetOptional lets you author in UMG or pure C++)
    // -----------------------------------------------------------------

    UPROPERTY(meta = (BindWidgetOptional))
    UListView* lst_roster = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    UButton* btn_add = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    UButton* btn_del = nullptr;

    // Local Apply/Cancel buttons in the dialog (optional).
    // If null, we fall back to UBaseScreen::ApplyButton/CancelButton.
    UPROPERTY(meta = (BindWidgetOptional))
    UButton* apply = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    UButton* cancel = nullptr;

    // Editable fields:
    UPROPERTY(meta = (BindWidgetOptional))
    UEditableTextBox* edt_name = nullptr;

    // Labels:
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* lbl_rank = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* lbl_flighttime = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* lbl_createdate = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* lbl_kills = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* lbl_deaths = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* lbl_missions = nullptr;

    // Images (rank/medal). In UMG, wrap with a transparent button if you want click handling.
    UPROPERTY(meta = (BindWidgetOptional))
    UImage* img_rank = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    UImage* img_medal = nullptr;

    // -----------------------------------------------------------------
    // State
    // -----------------------------------------------------------------

    UPROPERTY()
    TObjectPtr<UPlayerRosterItem> selected_item = nullptr;

    PlayerCharacter*              SelectedPlayer;

};
