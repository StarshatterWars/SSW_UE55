/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         MsnSelectDlg.h
    AUTHOR:       John DiCamillo

    UNREAL PORT:
    - Converted from FormWindow to UBaseScreen (UUserWidget).
    - Preserves original member names and intent.
    - Base class is UBaseScreen.
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"

// UMG:
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"

#include "MsnSelectDlg.generated.h"

class Campaign;
class Starshatter;

/**
 * Mission Select Dialog Active Window class (UE UUserWidget port)
 *
 * Notes:
 * - Starshatter ListBox/ComboBox are mapped to UE UListView/UComboBoxString.
 * - For UListView, you will supply UObject items (e.g., a simple UObject wrapper for campaign/mission rows)
 *   or use a custom entry widget + list item object.
 */
UCLASS()
class STARSHATTERWARS_API UMsnSelectDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UMsnSelectDlg(const FObjectInitializer& ObjectInitializer);

    // Original API surface (ported):
    virtual void RegisterControls();   // bind widget events, cache pointers if needed
    virtual void Show();               // make visible / focus
    virtual void ExecFrame();          // optional per-frame logic (prefer NativeOnKeyDown)

    // Operations (ported from AWEvent style; UE binds directly):
    UFUNCTION()
    virtual void OnCampaignSelect();

    UFUNCTION()
    virtual void OnMissionSelect();

    UFUNCTION()
    virtual void OnMod();

    UFUNCTION()
    virtual void OnNew();

    UFUNCTION()
    virtual void OnEdit();

    UFUNCTION()
    virtual void OnDel();

    UFUNCTION()
    virtual void OnDelConfirm();

    UFUNCTION()
    virtual void OnAccept();

    UFUNCTION()
    virtual void OnCancel();

protected:
    // UUserWidget lifecycle:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // Prefer key handling here rather than polling:
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

protected:
    // Starshatter: MenuScreen* manager;
    // UE: keep as UObject* (or replace with your concrete menu manager widget type later).
    UPROPERTY(BlueprintReadWrite, Category = "MsnSelectDlg")
    UObject* manager = nullptr;

    // Buttons:
    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UButton* btn_mod = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UButton* btn_new = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UButton* btn_edit = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UButton* btn_del = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UButton* btn_accept = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UButton* btn_cancel = nullptr;

    // Starshatter ComboBox/ListBox:
    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UComboBoxString* cmb_campaigns = nullptr;

    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UListView* lst_campaigns = nullptr;

    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UListView* lst_missions = nullptr;

    // Description text field:
    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UTextBlock* description = nullptr;

    // Model pointers:
    Starshatter* stars = nullptr;
    Campaign* campaign = nullptr;
    int32 selected_mission = -1;

    int32 mission_id = 0;
    bool editable = false;
};
