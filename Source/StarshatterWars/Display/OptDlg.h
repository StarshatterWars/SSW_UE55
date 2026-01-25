/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         OptDlg.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Main Menu Options Dialog (Unreal UUserWidget)
*/

#pragma once

// Minimal Unreal includes required by project conventions:
#include "Math/Vector.h"                // FVector
#include "Math/Color.h"                 // FColor
#include "Math/UnrealMathUtility.h"     // Math

#include "BaseScreen.h"
#include "OptDlg.generated.h"

// Forward declarations (keep header light):
class UButton;
class UComboBoxString;
class UTextBlock;
class UMenuScreen;

UCLASS()
class STARSHATTERWARS_API UOptDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UOptDlg(const FObjectInitializer& ObjectInitializer);

    // UUserWidget overrides:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
    // UE UMG focus + key handling:
    virtual bool NativeSupportsKeyboardFocus() const override { return true; }
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

public:
    // Operations:
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI")
    void Apply();

    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI")
    void Cancel();

    // Manager (MenuScreen owns dialog switching + apply/cancel options):
    void SetManager(UMenuScreen* InManager) { manager = InManager; }
    UMenuScreen* GetManager() const { return manager; }

protected:
    // Button handlers:
    UFUNCTION() void OnApplyClicked();
    UFUNCTION() void OnCancelClicked();

    // Combo change handlers (optional):
    UFUNCTION() void OnFlightModelChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnFlyingStartChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnLandingsChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnAIDifficultyChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnHudModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnHudColorChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnJoyModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnFfModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnGridModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnGunsightChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    // Navigation buttons:
    UFUNCTION() void OnAudioClicked();
    UFUNCTION() void OnVideoClicked();
    UFUNCTION() void OnOptionsClicked();
    UFUNCTION() void OnControlsClicked();
    UFUNCTION() void OnModClicked();

protected:
    UPROPERTY()
    UMenuScreen* manager = nullptr;

    // Option controls (legacy ComboBox -> Unreal UComboBoxString):
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* flight_model = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* flying_start = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* landings = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* ai_difficulty = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* hud_mode = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* hud_color = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* joy_mode = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* ff_mode = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* grid_mode = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* gunsight = nullptr;

    // Description area (legacy ActiveWindow -> UTextBlock placeholder):
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* description = nullptr;

    // Navigation buttons:
    UPROPERTY(meta = (BindWidgetOptional)) UButton* aud_btn = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* vid_btn = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* opt_btn = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* ctl_btn = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* mod_btn = nullptr;

    // Action buttons (naming convention):
    UPROPERTY(meta = (BindWidgetOptional)) UButton* ApplyBtn = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* CancelBtn = nullptr;

    bool closed = false;
};
