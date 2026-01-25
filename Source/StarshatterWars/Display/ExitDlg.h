/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         ExitDlg.h
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    Exit confirm dialog + rolling credits (Unreal port).

    UNREAL PORT:
    - UBaseScreen-derived (UUserWidget).
    - Uses centralized dialog input: HandleAccept/HandleCancel.
    - Preserves exit_latch and credits smooth scrolling.
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"

#include "ExitDlg.generated.h"

class UButton;
class URichTextBlock; // if you use UE's RichTextBlock; otherwise replace with your ported RichTextBox widget

// Forward declarations for ported core:
class MenuScreen;

UCLASS()
class STARSHATTERWARS_API UExitDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UExitDlg(const FObjectInitializer& ObjectInitializer);

    // UBaseScreen
    virtual void BindFormWidgets() override;

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // Centralized dialog actions (Enter/Escape):
    virtual void HandleAccept() override;
    virtual void HandleCancel() override;

public:
    // Legacy-style API surface (kept for parity with other ports):
    void RegisterControls();
    void Show();
    void ExecFrame();

    // Button handlers:
    UFUNCTION()
    void OnApply();

    UFUNCTION()
    void OnCancel();

protected:
    // Widgets (bind in UMG or via BindFormWidgets IDs):
    UPROPERTY(meta = (BindWidgetOptional)) UButton* btn_apply = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* btn_cancel = nullptr;

    // Credits control:
    // Option A: if you have a ported RichTextBox widget type, use that here.
    // Option B: if you are using UMG RichTextBlock, you'll need a different scrolling approach.
    // This assumes you have a ported URichTextBox-like widget with the same methods used below.
    UPROPERTY(meta = (BindWidgetOptional)) UObject* credits = nullptr;

    // Manager (owner screen/controller in your port):
    MenuScreen* manager = nullptr;

    // Classic state:
    bool bExitLatch = false;

    // Original def rect is a layout hint in classic UI; in UE you likely ignore or use for sizing.
    // Keeping for parity:
    // Rect def_rect;  // if you have a Rect type in your port layer
};
