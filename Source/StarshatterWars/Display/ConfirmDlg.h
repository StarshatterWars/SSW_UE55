/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         ConfirmDlg.h
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    UNREAL PORT:
    - Converted from FormWindow/AWEvent mapping to UBaseScreen (UUserWidget-derived).
    - Preserves legacy FORM IDs and behavior.
    - Enter/Escape unified via UBaseScreen::HandleAccept/HandleCancel.
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "ConfirmDlg.generated.h"

// UMG fwd:
class UButton;
class UTextBlock;

// Starshatter fwd:
class MenuScreen;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConfirmAccepted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConfirmCanceled);

UCLASS()
class STARSHATTERWARS_API UConfirmDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UConfirmDlg(const FObjectInitializer& ObjectInitializer);

    // ----------------------------------------------------------------
    // UUserWidget lifecycle
    // ----------------------------------------------------------------
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;

    // ----------------------------------------------------------------
    // Legacy surface (ported)
    // ----------------------------------------------------------------
    virtual void RegisterControls();
    virtual void Show();

    // Classic API
    UFUNCTION(BlueprintCallable, Category = "ConfirmDlg")
    FString GetTitle() const;

    UFUNCTION(BlueprintCallable, Category = "ConfirmDlg")
    void SetTitle(const FString& InTitle);

    UFUNCTION(BlueprintCallable, Category = "ConfirmDlg")
    FString GetMessage() const;

    UFUNCTION(BlueprintCallable, Category = "ConfirmDlg")
    void SetMessage(const FString& InMessage);

    // ----------------------------------------------------------------
    // Callbacks (UMG)
    // ----------------------------------------------------------------
    UFUNCTION()
    void OnApplyClicked();

    UFUNCTION()
    void OnCancelClicked();

    // ----------------------------------------------------------------
    // Events (replacement for parent_control->ClientEvent(EID_USER_1))
    // ----------------------------------------------------------------
    UPROPERTY(BlueprintAssignable, Category = "ConfirmDlg")
    FOnConfirmAccepted OnConfirmed;

    UPROPERTY(BlueprintAssignable, Category = "ConfirmDlg")
    FOnConfirmCanceled OnCanceled;

public:
    // Screen manager (set by owning menu screen)
    MenuScreen* manager = nullptr;

protected:
    // ----------------------------------------------------------------
    // UBaseScreen overrides
    // ----------------------------------------------------------------
    virtual void BindFormWidgets() override;
    virtual FString GetLegacyFormText() const override;

protected:
    // ----------------------------------------------------------------
    // Bound UMG controls (match FORM ids)
    // ----------------------------------------------------------------
    UPROPERTY(meta = (BindWidgetOptional)) UButton* btn_apply = nullptr; // id 1
    UPROPERTY(meta = (BindWidgetOptional)) UButton* btn_cancel = nullptr; // id 2

    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* lbl_title = nullptr; // id 100
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* lbl_message = nullptr; // id 101
};
