/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         FirstTimeDlg.h
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    First-time player setup screen.
    Unreal UMG version of the legacy FirstTimeDlg FormWindow.
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "FirstTimeDlg.generated.h"

// ------------------------------------------------------------
// Forward declarations (keep header light)
// ------------------------------------------------------------
class UButton;
class UEditableTextBox;
class UComboBoxString;
class UMenuScreen;

// ------------------------------------------------------------

UCLASS()
class STARSHATTERWARS_API UFirstTimeDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UFirstTimeDlg(const FObjectInitializer& ObjectInitializer);

    // --------------------------------------------------------
    // UUserWidget lifecycle
    // --------------------------------------------------------
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativeTick(
        const FGeometry& MyGeometry,
        float InDeltaTime
    ) override;

    // --------------------------------------------------------
    // Legacy parity
    // --------------------------------------------------------
    void ExecFrame();

    void SetManager(UMenuScreen* InManager);

protected:
    // --------------------------------------------------------
    // Button handlers
    // --------------------------------------------------------
    UFUNCTION()
    void OnApplyClicked();

protected:
    // --------------------------------------------------------
    // Bound UMG widgets
    // --------------------------------------------------------
    UPROPERTY(meta = (BindWidgetOptional))
    UEditableTextBox* NameEdit = nullptr;      // id 200

    UPROPERTY(meta = (BindWidgetOptional))
    UComboBoxString* PlaystyleCombo = nullptr; // id 201

    UPROPERTY(meta = (BindWidgetOptional))
    UComboBoxString* ExperienceCombo = nullptr; // id 202

protected:
    // --------------------------------------------------------
    // Owning menu screen
    // --------------------------------------------------------
    UMenuScreen* Manager = nullptr;
};
