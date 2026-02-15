/*=============================================================================
    Project:        Starshatter Wars
    Studio:         Fractal Dev Studios
    Copyright:      (c) 2025-2026.
    All Rights Reserved.

    SUBSYSTEM:      StarshatterWars (Unreal Engine)
    FILE:           JoyDlg.h
    AUTHOR:         Carlos Bott

    OVERVIEW
    ========
    UJoyDlg
    - Joystick axis mapping dialog (OptionsScreen subpage).
    - Lets player click an axis slot, then move a physical axis to detect it.
    - Writes bindings to legacy KeyMap (key.cfg) and calls Stars->MapKeys().
    - Hosted and routed by UOptionsScreen (single source of truth).

    NOTES
    =====
    - Uses explicit UTextBlock bindings for axis labels (no widget tree probing).
    - Invert is a UCheckBox per axis.
    - Uses AddUniqueDynamic bindings (no RemoveAll) to avoid ensure crashes.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"

#include "JoyDlg.generated.h"

// Forward declarations:
class UButton;
class UTextBlock;
class UCheckBox;
class UOptionsScreen;

UCLASS()
class STARSHATTERWARS_API UJoyDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UJoyDlg(const FObjectInitializer& ObjectInitializer);

    // Standardized across subscreens:
    void SetOptionsManager(UOptionsScreen* InManager) { OptionsManager = InManager; }
    UOptionsScreen* GetOptionsManager() const { return OptionsManager.Get(); }

    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // BaseScreen pattern:
    virtual void ExecFrame(double DeltaTime) override;

    void Show();
    void Apply();
    void Cancel();

protected:
    // Apply/Cancel
    UFUNCTION() void OnApplyClicked();
    UFUNCTION() void OnCancelClicked();

    // Axis slots
    UFUNCTION() void OnAxis0Clicked();
    UFUNCTION() void OnAxis1Clicked();
    UFUNCTION() void OnAxis2Clicked();
    UFUNCTION() void OnAxis3Clicked();

private:
    void BindDelegates();

    void HandleAxisClicked(int32 AxisIndex);
    void RefreshAxisUIFromCurrentBindings();
    void SetAxisLabel(int32 AxisIndex, const TCHAR* Text);

    void CommitToKeyMap();

private:
    bool bDelegatesBound = false;

    int32 SelectedAxis = -1;   // which slot (0..3) is waiting for input
    int32 SampleAxis = -1;     // last detected physical axis (0..7)

    int32 Samples[8] = { 0,0,0,0,0,0,0,0 };
    int32 MapAxis[4] = { -1,-1,-1,-1 }; // slot->physical axis (0..7), -1 unmapped

protected:
    // Optional: status text
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> MessageText = nullptr;

    // Axis buttons
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> AxisButton0 = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> AxisButton1 = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> AxisButton2 = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> AxisButton3 = nullptr;

    // Axis labels (bind these in the WBP; DO NOT rely on button children)
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> AxisText0 = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> AxisText1 = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> AxisText2 = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> AxisText3 = nullptr;

    // Invert checkboxes (recommended)
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UCheckBox> Invert0 = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UCheckBox> Invert1 = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UCheckBox> Invert2 = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UCheckBox> Invert3 = nullptr;

    // Buttons
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> ApplyBtn = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> CancelBtn = nullptr;
};
