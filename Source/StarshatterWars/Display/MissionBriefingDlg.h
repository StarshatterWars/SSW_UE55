/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         MissionBriefingDlg.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UMissionBriefingDlg
    - Unreal UUserWidget replacement for legacy MsnDlg.
    - Inherits from UBaseScreen to use legacy FORM parsing.
    - Implements mission briefing dialog behavior.
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"

#include "MissionBriefingDlg.generated.h"

class UPlanScreen;
class Campaign;
class Mission;
class MissionInfo;

UCLASS()
class STARSHATTERWARS_API UMissionBriefingDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UMissionBriefingDlg();
    void InitializeMissionBriefing(UPlanScreen* InPlanScreen);
    void ShowMsnDlg();

protected:
    int32 CalcTimeOnTarget() const;

protected:
    // UBaseScreen overrides
    virtual void BindFormWidgets() override;
    virtual FString GetLegacyFormText() const override;

protected:
    // UUserWidget lifecycle
    virtual void NativeConstruct() override;

protected:
    // Button handlers
    UFUNCTION() void OnCommitClicked();
    UFUNCTION() void OnCancelClicked();
    UFUNCTION() void OnSitClicked();
    UFUNCTION() void OnPkgClicked();
    UFUNCTION() void OnNavClicked();
    UFUNCTION() void OnWepClicked();

protected:
    // Legacy FORM text
    UPROPERTY(EditDefaultsOnly, Category = "FORM")
    FString LegacyFormText;

protected:
    UPlanScreen* PlanScreen = nullptr;

    Campaign* campaign = nullptr;
    Mission* mission = nullptr;
    MissionInfo* info = nullptr;

    int32 pkg_index = -1;

protected:
    // Bound widgets (FORM IDs)
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* MissionName = nullptr;            // id 200
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* MissionSystem = nullptr;          // id 202
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* MissionSector = nullptr;          // id 204
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* MissionTimeStart = nullptr;       // id 206
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* MissionTimeTarget = nullptr;      // id 208
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* MissionTimeTargetLabel = nullptr; // id 207

    UPROPERTY(meta = (BindWidgetOptional)) UButton* SitButton = nullptr;   // id 900
    UPROPERTY(meta = (BindWidgetOptional)) UButton* PkgButton = nullptr;   // id 901
    UPROPERTY(meta = (BindWidgetOptional)) UButton* NavButton = nullptr;   // id 902
    UPROPERTY(meta = (BindWidgetOptional)) UButton* WepButton = nullptr;   // id 903

    UPROPERTY(meta = (BindWidgetOptional)) UButton* CommitButton = nullptr; // id 1
    UPROPERTY(meta = (BindWidgetOptional)) UButton* CancelBtn = nullptr;    // id 2
};
#pragma once
