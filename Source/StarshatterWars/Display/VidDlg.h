/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         VidDlg.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Main Menu Video Dialog (Unreal UUserWidget)
*/

#pragma once

// Minimal Unreal includes required by project conventions:
#include "Math/Vector.h"                // FVector
#include "Math/Color.h"                 // FColor
#include "Math/UnrealMathUtility.h"     // Math

#include "BaseScreen.h"
#include "VidDlg.generated.h"

// Forward declarations (keep header light):
class UButton;
class UComboBoxString;
class USlider;
class UMenuScreen;

UCLASS()
class STARSHATTERWARS_API UVidDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UVidDlg(const FObjectInitializer& ObjectInitializer);

    // UUserWidget overrides:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
    // UE UMG focus + key handling:
    virtual bool NativeSupportsKeyboardFocus() const override { return true; }
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

public:
    // Operations (legacy callbacks preserved as UFUNCTION handlers):
    UFUNCTION() void OnTexSizeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnDetailChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnTextureChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnGammaChanged(float Value);

    // Apply/Cancel:
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI")
    void Apply();

    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI")
    void Cancel();

    // Manager (MenuScreen owns dialog switching + apply/cancel options):
    void SetManager(UMenuScreen* InManager) { manager = InManager; }
    UMenuScreen* GetManager() const { return manager; }

protected:
    // Click handlers:
    UFUNCTION() void OnApplyClicked();
    UFUNCTION() void OnCancelClicked();

    // Tab navigation:
    UFUNCTION() void OnAudioClicked();
    UFUNCTION() void OnVideoClicked();
    UFUNCTION() void OnOptionsClicked();
    UFUNCTION() void OnControlsClicked();
    UFUNCTION() void OnModClicked();

protected:
    // Internal helpers:
    void BuildModeList();

protected:
    UPROPERTY()
    UMenuScreen* manager = nullptr;

    // Option controls (legacy ComboBox/Slider -> Unreal UComboBoxString/USlider):
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* mode = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* tex_size = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* detail = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* texture = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* shadows = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* bump_maps = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* spec_maps = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* lens_flare = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* corona = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* nebula = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* dust = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) USlider* gamma = nullptr;

    // Navigation buttons:
    UPROPERTY(meta = (BindWidgetOptional)) UButton* aud_btn = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* vid_btn = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* opt_btn = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* ctl_btn = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* mod_btn = nullptr;

    // Action buttons:
    UPROPERTY(meta = (BindWidgetOptional)) UButton* ApplyBtn = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* CancelBtn = nullptr;

    // Legacy selection tracking:
    int selected_render = 0;
    int selected_card = 0;
    int selected_tex_size = 0;
    int selected_mode = 0;
    int selected_detail = 0;
    int selected_texture = 0;
    int orig_gamma = 0;

    bool closed = false;
};
