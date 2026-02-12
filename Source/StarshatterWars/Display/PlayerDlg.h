/*=============================================================================
    Project:        Starshatter Wars (nGenEx Unreal Port)
    Studio:         Fractal Dev Games
    Copyright:      (C) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:      Stars.exe
    FILE:           PlayerDlg.h
    AUTHOR:         Carlos Bott

    OVERVIEW
    ========
    UPlayerDlg

    Unreal UMG replacement for legacy PlayerDlg.

    PORT NOTES
    ==========
    - PlayerCharacter has been fully removed.
    - UStarshatterPlayerSubsystem is the authoritative profile owner.
    - FS_PlayerGameInfo is the data model.
    - Current implementation supports a single-profile save slot.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "PlayerDlg.generated.h"

class UButton;
class UEditableTextBox;
class UImage;
class UListView;
class UTextBlock;
class UMenuScreen;
class UPlayerRosterItem;
class UStarshatterPlayerSubsystem;

UCLASS()
class STARSHATTERWARS_API UPlayerDlg : public UBaseScreen
{
    GENERATED_BODY()

public:

    // ------------------------------------------------------------------
    // Construction
    // ------------------------------------------------------------------

    UPlayerDlg(const FObjectInitializer& ObjectInitializer);

    void InitializeDlg(UMenuScreen* InManager);

    // ------------------------------------------------------------------
    // UUserWidget overrides
    // ------------------------------------------------------------------

    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual FReply NativeOnKeyDown(
        const FGeometry& InGeometry,
        const FKeyEvent& InKeyEvent) override;

    // ------------------------------------------------------------------
    // Dialog visibility
    // ------------------------------------------------------------------

    void ShowDlg();
    void HideDlg();

protected:

    // ------------------------------------------------------------------
    // Control registration
    // ------------------------------------------------------------------

    void RegisterControls();

    // ------------------------------------------------------------------
    // Subsystem access
    // ------------------------------------------------------------------

    UStarshatterPlayerSubsystem* GetPlayerSubsystem() const;

    // ------------------------------------------------------------------
    // Roster handling
    // ------------------------------------------------------------------

    void BuildRoster();
    void RefreshRoster();
    void OnRosterSelectionChanged(UObject* SelectedItem);

    const UPlayerRosterItem* GetSelectedItem() const;

    // ------------------------------------------------------------------
    // UI <-> Model
    // ------------------------------------------------------------------

    void UpdatePlayer();
    void ShowPlayer();

    // ------------------------------------------------------------------
    // Actions
    // ------------------------------------------------------------------

    UFUNCTION()
    void OnAdd();

    UFUNCTION()
    void OnDel();

    UFUNCTION()
    void OnApply();

    UFUNCTION()
    void OnCancel();

    // ------------------------------------------------------------------
    // Formatting helpers
    // ------------------------------------------------------------------

    FString FormatTimeHMS(double Seconds);
    FString FormatDateFromUnixSeconds(int64 UnixSeconds);

protected:

    // ------------------------------------------------------------------
    // Bound widgets
    // ------------------------------------------------------------------

    UPROPERTY(meta = (BindWidget))
    UListView* lst_roster;

    UPROPERTY(meta = (BindWidget))
    UEditableTextBox* edt_name;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* lbl_rank;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* lbl_flighttime;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* lbl_createdate;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* lbl_kills;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* lbl_deaths;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* lbl_missions;

    UPROPERTY(meta = (BindWidgetOptional))
    UImage* img_rank;

    UPROPERTY(meta = (BindWidgetOptional))
    UImage* img_medal;

    UPROPERTY(meta = (BindWidgetOptional))
    UButton* btn_add;

    UPROPERTY(meta = (BindWidgetOptional))
    UButton* btn_del;

    UPROPERTY(meta = (BindWidgetOptional))
    UButton* apply;

    UPROPERTY(meta = (BindWidgetOptional))
    UButton* cancel;

private:

    // ------------------------------------------------------------------
    // Internal state
    // ------------------------------------------------------------------

    UPROPERTY()
    UPlayerRosterItem* selected_item = nullptr;

    UPROPERTY()
    UMenuScreen* manager = nullptr;
};
