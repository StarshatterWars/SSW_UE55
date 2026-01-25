/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         ExitDlg.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Exit / Credits Dialog (Unreal UUserWidget)
*/

#pragma once

// Minimal Unreal includes required by project conventions:
#include "Math/Vector.h"                // FVector
#include "Math/Color.h"                 // FColor
#include "Math/UnrealMathUtility.h"     // Math

#include "Blueprint/UserWidget.h"
#include "ExitDlg.generated.h"

// Forward declarations (keep header light):
class UButton;
class UTextBlock;
class UMultiLineEditableTextBox;
class UBaseScreen;

UCLASS()
class STARSHATTERWARS_API UExitDlg : public UUserWidget
{
    GENERATED_BODY()

public:
    UExitDlg(const FObjectInitializer& ObjectInitializer);

    // Manager bridge (typically the Menu Screen widget/controller):
    void SetManager(UBaseScreen* InManager) { manager = InManager; }
    UBaseScreen* GetManager() const { return manager; }

    // UUserWidget overrides:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
    virtual bool IsFocusable() const override { return true; }

    // Keyboard handling (Enter = Apply/Exit, Escape = Cancel):
    virtual FReply NativeOnKeyDown(
        const FGeometry& InGeometry,
        const FKeyEvent& InKeyEvent) override;

protected:
    // Operations:
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI")
    void Apply();

    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI")
    void Cancel();

protected:
    // Button handlers:
    UFUNCTION() void OnApplyClicked();
    UFUNCTION() void OnCancelClicked();

protected:
    // External dialog manager (legacy bridge):
    UPROPERTY() UBaseScreen* manager = nullptr;

    // Credits text (RichTextBox -> UMG equivalent):
    // If you want true rich text markup, switch this to URichTextBlock.
    UPROPERTY(meta = (BindWidgetOptional)) UMultiLineEditableTextBox* credits = nullptr;

    // Action buttons:
    UPROPERTY(meta = (BindWidgetOptional)) UButton* ApplyBtn = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* CancelBtn = nullptr;

    // Legacy state:
    bool exit_latch = false;
};

