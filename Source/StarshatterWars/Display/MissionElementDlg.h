/*  Project Starshatter Wars
    Fractal Dev Studios LLC
    Copyright (c) 2025-2026

    SUBSYSTEM:    Stars.exe (Unreal Port)
    FILE:         MissionElementDlg.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UMissionElementDlg
    - Unreal UUserWidget replacement for legacy MsnElemDlg.
    - Uses standard UMG widget bindings (BindWidgetOptional).
    - Bindings correspond to MsnElemDlg.frm control IDs.
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "MissionElementDlg.generated.h"

class UButton;
class UComboBoxString;
class UEditableTextBox;
class UTextBlock;

class UMenuScreen;            // your UE screen/manager wrapper
class UMsnEditDlg;            // optional UE wrapper
class Mission;
class MissionElement;

UCLASS()
class STARSHATTERWARS_API UMissionElementDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UMissionElementDlg(const FObjectInitializer& ObjectInitializer);

    void InitializeDlg(UMenuScreen* InManager);
    void SetMission(Mission* InMission);
    void SetMissionElement(MissionElement* InElem);
    void ShowDlg();

protected:
    virtual void NativeConstruct() override;

private:
    void RebuildFromModel();          // fills UI from elem/mission
    void UpdateTeamInfo();            // commander/squadron/carrier lists
    void BuildObjectiveTargets();     // based on objective selection
    void RebuildDesignListFromClass();
    void RebuildSkinAndLoadoutFromDesign();

    bool CanCommand(const MissionElement* Commander, const MissionElement* Subordinate) const;

private:
    // UMG event handlers
    UFUNCTION() void OnAcceptClicked();
    UFUNCTION() void OnCancelClicked();

    UFUNCTION() void OnClassChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnDesignChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnObjectiveChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION() void OnIFFCommitted(const FText& Text, ETextCommit::Type CommitMethod);

private:
    UPROPERTY(Transient)
    UMenuScreen* Manager = nullptr;

    Mission* MissionPtr = nullptr;
    MissionElement* ElemPtr = nullptr;

    bool bExitLatch = true;

    // -----------------------------
    // Widget bindings (OPTIONAL)
    // -----------------------------

    // Left column
    UPROPERTY(meta = (BindWidgetOptional)) UEditableTextBox* NameEdit = nullptr;      // id 201
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* ClassCombo = nullptr;    // id 202
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* DesignCombo = nullptr;   // id 203
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* SkinCombo = nullptr;     // id 213
    UPROPERTY(meta = (BindWidgetOptional)) UEditableTextBox* SizeEdit = nullptr;      // id 204
    UPROPERTY(meta = (BindWidgetOptional)) UEditableTextBox* IFFEdit = nullptr;       // id 205
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* RoleCombo = nullptr;     // id 206
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* RegionCombo = nullptr;   // id 207

    UPROPERTY(meta = (BindWidgetOptional)) UEditableTextBox* LocXEdit = nullptr;      // id 208
    UPROPERTY(meta = (BindWidgetOptional)) UEditableTextBox* LocYEdit = nullptr;      // id 209
    UPROPERTY(meta = (BindWidgetOptional)) UEditableTextBox* LocZEdit = nullptr;      // id 210

    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* HeadingCombo = nullptr;  // id 211
    UPROPERTY(meta = (BindWidgetOptional)) UEditableTextBox* HoldTimeEdit = nullptr;  //*

    // OK/Cancel
    UPROPERTY(meta = (BindWidgetOptional)) UButton* AcceptButton = nullptr; // 1
};