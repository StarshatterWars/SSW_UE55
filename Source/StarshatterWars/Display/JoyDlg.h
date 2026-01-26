/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         JoyDlg.h
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    Joystick Axis Setup dialog.
    Unreal UMG version of the legacy JoyDlg FormWindow.
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "MenuScreen.h"


// Minimal Unreal includes requested for headers:
#include "Math/Vector.h"              // FVector
#include "Math/Color.h"               // FColor
#include "Math/UnrealMathUtility.h"   // Math

#include "JoyDlg.generated.h"

// ------------------------------------------------------------
// Forward declarations (keep header light)
// ------------------------------------------------------------
class UButton;
class UTextBlock;
class UComboBoxString;

// ------------------------------------------------------------

UCLASS()
class STARSHATTERWARS_API UJoyDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UJoyDlg(const FObjectInitializer& ObjectInitializer);

    // UUserWidget lifecycle:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // Legacy parity:
    void ExecFrame();
    void SetManager(UMenuScreen* InManager);

protected:
    // UMG handlers:
    UFUNCTION()
    void OnApplyClicked();

    UFUNCTION()
    void OnCancelClicked();

    UFUNCTION()
    void OnAxis0Clicked();

    UFUNCTION()
    void OnAxis1Clicked();

    UFUNCTION()
    void OnAxis2Clicked();

    UFUNCTION()
    void OnAxis3Clicked();

protected:
    // --------------------------------------------------------
    // Bound UMG widgets (BindWidgetOptional to allow iteration)
    // --------------------------------------------------------
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* MessageText = nullptr;     // id 11 (label)

    UPROPERTY(meta = (BindWidgetOptional))
    UButton* AxisButton0 = nullptr;        // id 201

    UPROPERTY(meta = (BindWidgetOptional))
    UButton* AxisButton1 = nullptr;        // id 202

    UPROPERTY(meta = (BindWidgetOptional))
    UButton* AxisButton2 = nullptr;        // id 203

    UPROPERTY(meta = (BindWidgetOptional))
    UButton* AxisButton3 = nullptr;        // id 204

    UPROPERTY(meta = (BindWidgetOptional))
    UButton* Invert0 = nullptr;            // id 301

    UPROPERTY(meta = (BindWidgetOptional))
    UButton* Invert1 = nullptr;            // id 302

    UPROPERTY(meta = (BindWidgetOptional))
    UButton* Invert2 = nullptr;            // id 303

    UPROPERTY(meta = (BindWidgetOptional))
    UButton* Invert3 = nullptr;            // id 304

protected:
    // Manager screen:
    UMenuScreen* Manager = nullptr;

    // Local state (replaces legacy statics):
    int SelectedAxis = -1;
    int SampleAxis = -1;
    int Samples[8] = { 0,0,0,0,0,0,0,0 };
    int MapAxis[4] = { -1,-1,-1,-1 };

private:
    // Helpers:
    void HandleAxisClicked(int AxisIndex);
    void RefreshAxisButtonsFromCurrentBindings();
    void UpdateAxisButtonText(int AxisIndex, const char* TextId);
    void SetInvertButtonState(UButton* InvertButton, bool bChecked);
    bool GetInvertButtonState(const UButton* InvertButton) const;
};
