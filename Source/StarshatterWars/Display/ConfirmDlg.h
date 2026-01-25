/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         ConfirmDlg.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    General-purpose confirmation dialog (Unreal port).

    UNREAL PORT:
    - Converted from FormWindow/AWEvent mapping to UBaseScreen (UUserWidget-derived).
    - Keeps legacy API surface: SetTitle/SetMessage, parent_control callback concept.
    - Replaces ClientEvent(EID_USER_1) with an explicit delegate/callback.
    - Removes MemDebug and allocation tags.
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

#include "ConfirmDlg.generated.h"

class MenuScreen;

/**
 * Confirm dialog:
 * - Two buttons (Apply/Cancel)
 * - Title + message text blocks
 * - Optional callback when Apply is pressed (legacy EID_USER_1 equivalent)
 */
UCLASS()
class STARSHATTERWARS_API UConfirmDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UConfirmDlg(const FObjectInitializer& ObjectInitializer);

    // ---- UBaseScreen -------------------------------------------------
    virtual void BindFormWidgets() override;
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

public:
    // ---- Legacy-ish API ---------------------------------------------

    // In classic: parent_control->ClientEvent(EID_USER_1) on Apply.
    // In UE: store a lightweight callback to execute on Apply.
    void SetOnApply(TFunction<void()> InOnApply);

    FText GetTitle() const;
    void  SetTitle(const FText& InTitle);

    FText GetMessage() const;
    void  SetMessage(const FText& InMessage);

    void Show(); // Optional convenience; actual visibility is your manager's job.

protected:
    void RegisterControls();
    void ExecFrame();

    UFUNCTION()
    void OnApply();

    UFUNCTION()
    void OnCancel();

protected:
    // UMG bindings (use BindWidgetOptional so you can evolve layouts safely):
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* lbl_title = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* lbl_message = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UButton* btn_apply = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* btn_cancel = nullptr;

protected:
    // Not a UObject (legacy screen manager):
    MenuScreen* manager = nullptr;

    // Apply callback:
    TFunction<void()> OnApplyCallback;

    // Simple latch so Enter/Escape don't double-fire in the same press frame:
    bool bExitLatch = true;
};
