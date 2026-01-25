/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         ConfirmDlg.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    General-purpose confirmation dialog (Unreal UUserWidget)
*/

#pragma once

// Minimal Unreal includes required by project conventions:
#include "Math/Vector.h"                // FVector
#include "Math/Color.h"                 // FColor
#include "Math/UnrealMathUtility.h"     // Math

#include "Blueprint/UserWidget.h"
#include "ConfirmDlg.generated.h"

// Forward declarations (keep header light):
class UButton;
class UTextBlock;
class UWidget;
class UBaseScreen;

UCLASS()
class STARSHATTERWARS_API UConfirmDlg : public UUserWidget
{
    GENERATED_BODY()

public:
    UConfirmDlg(const FObjectInitializer& ObjectInitializer);

    // Manager bridge (typically the Menu Screen widget/controller):
    void SetManager(UBaseScreen* InManager) { manager = InManager; }
    UBaseScreen* GetManager() const { return manager; }

    // Parent control bridge (the widget that launched the confirm dialog):
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI")
    UWidget* GetParentControl() const { return parent_control; }

    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI")
    void SetParentControl(UWidget* p) { parent_control = p; }

    // Title / message (mirrors legacy Text interface):
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI")
    FText GetTitleText() const { return title_text; }

    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI")
    void SetTitleText(const FText& t);

    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI")
    FText GetMessageText() const { return message_text; }

    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI")
    void SetMessageText(const FText& m);

    // UUserWidget overrides:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
    virtual bool IsFocusable() const override { return true; }

    // Keyboard handling (Enter = Apply, Escape = Cancel):
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

    // Legacy concept of parent control (any widget can be the source):
    UPROPERTY() UWidget* parent_control = nullptr;

    // Cached text values:
    UPROPERTY() FText title_text;
    UPROPERTY() FText message_text;

    // Widgets (BindWidgetOptional keeps this header usable across WBP variants):
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* lbl_title = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* lbl_message = nullptr;

    // Buttons:
    UPROPERTY(meta = (BindWidgetOptional)) UButton* ApplyBtn = nullptr;   // legacy: btn_apply
    UPROPERTY(meta = (BindWidgetOptional)) UButton* CancelBtn = nullptr;  // legacy: btn_cancel
};
